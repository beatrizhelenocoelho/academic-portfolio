import pygame
import numpy as np
import cv2
import math
import sys
import os
import threading
from datetime import datetime

# ======================
# Parâmetros da Simulação
# ======================
PIXELS_PER_METER = 50
LIDAR_RANGE_M = 4
LIDAR_RANGE_PX = int(LIDAR_RANGE_M * PIXELS_PER_METER)
LIDAR_FOV = 150
LIDAR_RES = 1
LIDAR_NOISE_STD = 1.0

ODOM_NOISE_LINEAR = 0.07
ODOM_NOISE_ANGULAR = 0.07
INITIAL_POSITION_UNCERTAINTY = 10.0
MAX_UNCERTAINTY = 50

L_MIN = -4.0
L_MAX = 4.0
L_OCC = 0.8
L_FREE = -0.8

WINDOW_WIDTH = 1400
WINDOW_HEIGHT = 900
INFO_PANEL_WIDTH = 350
MAP_VIEW_WIDTH = WINDOW_WIDTH - INFO_PANEL_WIDTH
MAP_VIEW_HEIGHT = WINDOW_HEIGHT
FONT_SIZE = 24
SMALL_FONT_SIZE = 18

ROBOT_RADIUS = 1

CAMERA_SMOOTHING = 0.15

# Parâmetro para dilatação dos limites (kernel)
BORDER_KERNEL_SIZE = 3  # Ajuste para controlar a espessura da dilatação
# Parâmetro para o threshold dos free-space (áreas brancas)
CONTOUR_THRESHOLD = 200
# Espessura da borda preta a ser desenhada
BORDER_THICKNESS = 2

# ---------------------------------------------------------------------------
# Função para carregar o mapa e adicionar padding (margem extra)
# ---------------------------------------------------------------------------
def load_and_pad_map(map_path, pad=150):
    raw_map = cv2.imread(map_path, cv2.IMREAD_GRAYSCALE)
    if raw_map is None:
        print("Erro: Não foi possível carregar o mapa!")
        sys.exit(1)
    # Adiciona padding com valor 255 (assumindo que 255 indica área livre)
    padded_map = np.pad(raw_map, pad_width=pad, mode='constant', constant_values=255)
    return padded_map

# ---------------------------------------------------------------------------
# Funções auxiliares para colisão, posicionamento e mapeamento
# ---------------------------------------------------------------------------
def check_collision(sim_map, x, y, radius=ROBOT_RADIUS):
    map_height, map_width = sim_map.shape
    if x - radius < 0 or x + radius >= map_width or y - radius < 0 or y + radius >= map_height:
        return True
    for dx in range(-radius, radius + 1):
        for dy in range(-radius, radius + 1):
            if dx*dx + dy*dy <= radius*radius:
                check_x = int(x + dx)
                check_y = int(y + dy)
                if (0 <= check_x < map_width and 0 <= check_y < map_height and 
                    sim_map[check_y, check_x] <= 127):
                    return True
    return False

def find_valid_position(sim_map, start_x, start_y, target_x, target_y, radius=ROBOT_RADIUS):
    if not check_collision(sim_map, target_x, target_y, radius):
        return target_x, target_y
    dx = target_x - start_x
    dy = target_y - start_y
    distance = math.sqrt(dx*dx + dy*dy)
    if distance == 0:
        return start_x, start_y
    dx_norm = dx / distance
    dy_norm = dy / distance
    for step in range(int(distance)):
        test_x = start_x + step * dx_norm
        test_y = start_y + step * dy_norm
        if check_collision(sim_map, test_x, test_y, radius):
            if step == 0:
                return start_x, start_y
            else:
                return start_x + (step - 1) * dx_norm, start_y + (step - 1) * dy_norm
    return target_x, target_y

def save_final_map(occupancy_grid, output_folder="output_maps"):
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    # Converte o grid de ocupação para imagem (valores entre 0 e 255)
    probs = 1.0 / (1.0 + np.exp(-occupancy_grid))
    map_img = (255 * (1.0 - probs)).astype(np.uint8)
    # Aplicar dilatação para engrossar os limites
    kernel = np.ones((BORDER_KERNEL_SIZE, BORDER_KERNEL_SIZE), np.uint8)
    map_img = cv2.dilate(map_img, kernel, iterations=1)
    # Agora, gere uma máscara binária para encontrar os contornos
    ret, bin_map = cv2.threshold(map_img, CONTOUR_THRESHOLD, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(bin_map.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    # Desenha os contornos (bordas) em preto (valor 0)
    cv2.drawContours(map_img, contours, -1, (0,), thickness=BORDER_THICKNESS)
    # Gera nome do arquivo com timestamp e salva
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"final_map_{timestamp}.png"
    filepath = os.path.join(output_folder, filename)
    cv2.imwrite(filepath, map_img)
    print(f"Mapa final salvo em: {filepath}")
    return filepath

def create_ground_truth_reference(original_map):
    reference = np.zeros_like(original_map, dtype=np.float32)
    reference[original_map > 127] = 0.0  # espaço livre
    reference[original_map <= 127] = 1.0  # obstáculo
    return reference

def find_free_starting_position(sim_map):
    height, width = sim_map.shape
    center = np.array([height / 2, width / 2])
    free_pixels = np.argwhere(sim_map > 127)
    if free_pixels.size == 0:
        return (width // 2, height // 2)
    distances = np.linalg.norm(free_pixels - center, axis=1)
    idx = np.argmin(distances)
    pos_y, pos_x = free_pixels[idx]
    return (int(pos_x), int(pos_y))

def update_realistic_grid(grid, hits, rays):
    L_OCC_STRONG = 1.2
    L_OCC_WEAK = 0.6
    L_FREE_STRONG = -1.0
    L_FREE_WEAK = -0.4
    for ray_start, ray_end in rays:
        ray_points = bresenham(ray_start[0], ray_start[1], ray_end[0], ray_end[1])
        for i, (x, y) in enumerate(ray_points[:-1]):  # ignora o ponto de impacto
            if 0 <= x < grid.shape[1] and 0 <= y < grid.shape[0]:
                confidence = max(0.3, 1.0 - i / len(ray_points))
                update_val = L_FREE_STRONG if confidence > 0.7 else L_FREE_WEAK
                variation = np.random.uniform(0.95, 1.05)
                grid[y, x] = np.clip(grid[y, x] + update_val * variation, L_MIN, L_MAX)
    for hx, hy in hits:
        if 0 <= hx < grid.shape[1] and 0 <= hy < grid.shape[0]:
            neighbors_occupied = 0
            neighbors_total = 0
            for dx in [-1, 0, 1]:
                for dy in [-1, 0, 1]:
                    nx, ny = hx + dx, hy + dy
                    if 0 <= nx < grid.shape[1] and 0 <= ny < grid.shape[0]:
                        neighbors_total += 1
                        if grid[ny, nx] > 0:
                            neighbors_occupied += 1
            consistency = neighbors_occupied / neighbors_total if neighbors_total > 0 else 0.5
            update_val = L_OCC_STRONG if consistency > 0.6 else L_OCC_WEAK
            variation = np.random.uniform(0.9, 1.1)
            grid[hy, hx] = np.clip(grid[hy, hx] + update_val * variation, L_MIN, L_MAX)

# ---------------------------------------------------------------------------
# Classe de incerteza da pose
# ---------------------------------------------------------------------------
class PoseUncertainty:
    def __init__(self, initial_pos, initial_uncertainty=INITIAL_POSITION_UNCERTAINTY):
        self.true_pos = np.array(initial_pos, dtype=float)
        self.estimated_pos = np.array(initial_pos, dtype=float)
        self.true_angle = 0.0
        self.estimated_angle = 0.0
        self.covariance = np.eye(2) * (initial_uncertainty ** 2)
        
    def update_pose(self, dx, dy, dtheta, sim_map):
        update_realistic_pose(self, dx, dy, dtheta, sim_map)
        
    def get_uncertainty_ellipse(self, confidence=0.95):
        chi2_val = 9.21
        eigenvals, eigenvecs = np.linalg.eigh(self.covariance)
        angle = np.arctan2(eigenvecs[1, 0], eigenvecs[0, 0])
        width = 2 * np.sqrt(chi2_val * eigenvals[0])
        height = 2 * np.sqrt(chi2_val * eigenvals[1])
        return width, height, np.degrees(angle)

def update_realistic_pose(self, dx, dy, dtheta, sim_map):
    new_true_pos = self.true_pos + np.array([dx, dy])
    # Verifica colisão e ajusta se necessário
    valid_x, valid_y = find_valid_position(sim_map, self.true_pos[0], self.true_pos[1], new_true_pos[0], new_true_pos[1])
    self.true_pos = np.array([valid_x, valid_y])
    self.true_angle += dtheta
    systematic_error_x = dx * 0.1
    systematic_error_y = dy * 0.1
    systematic_error_theta = dtheta * 0.1
    distance_moved = np.sqrt(dx**2 + dy**2)
    random_noise_std = ODOM_NOISE_LINEAR * distance_moved
    angular_noise_std = ODOM_NOISE_ANGULAR * abs(dtheta) * 0.1
    noise_x = np.random.normal(systematic_error_x, random_noise_std)
    noise_y = np.random.normal(systematic_error_y, random_noise_std)
    noise_theta = np.random.normal(systematic_error_theta, angular_noise_std)
    self.estimated_pos += np.array([dx + noise_x, dy + noise_y])
    self.estimated_angle += dtheta + noise_theta
    movement_cov = np.eye(2) * (random_noise_std ** 2)
    rotation_cov = np.eye(2) * (abs(dtheta) * 0.5) ** 2
    self.covariance += movement_cov + rotation_cov
    eigenvals, eigenvecs = np.linalg.eigh(self.covariance)
    eigenvals = np.clip(eigenvals, 1.0, MAX_UNCERTAINTY**2)
    self.covariance = eigenvecs @ np.diag(eigenvals) @ eigenvecs.T

# ---------------------------------------------------------------------------
# Classe Camera para visualização
# ---------------------------------------------------------------------------
class Camera:
    def __init__(self, map_width, map_height):
        self.map_width = map_width
        self.map_height = map_height
        self.x = (map_width - MAP_VIEW_WIDTH) / 2.0
        self.y = (map_height - MAP_VIEW_HEIGHT) / 2.0
        self.target_x = self.x
        self.target_y = self.y
        
    def update(self, robot_pos):
        pos = robot_pos.estimated_pos
        self.target_x = pos[0] - MAP_VIEW_WIDTH // 2
        self.target_y = pos[1] - MAP_VIEW_HEIGHT // 2
        self.target_x = max(0, min(self.target_x, self.map_width - MAP_VIEW_WIDTH))
        self.target_y = max(0, min(self.target_y, self.map_height - MAP_VIEW_HEIGHT))
        self.x += (self.target_x - self.x) * CAMERA_SMOOTHING
        self.y += (self.target_y - self.y) * CAMERA_SMOOTHING
        
    def world_to_screen(self, world_pos):
        return (int(world_pos[0] - self.x), int(world_pos[1] - self.y))
    
    def screen_to_world(self, screen_pos):
        return (int(screen_pos[0] + self.x), int(screen_pos[1] + self.y))

# ---------------------------------------------------------------------------
# Funções de desenho
# ---------------------------------------------------------------------------
def draw_uncertainty_ellipse(surface, pose_uncertainty, camera, color=(255, 255, 0), confidence=0.95):
    width, height, angle = pose_uncertainty.get_uncertainty_ellipse(confidence)
    center_screen = camera.world_to_screen(pose_uncertainty.estimated_pos)
    if (0 <= center_screen[0] <= MAP_VIEW_WIDTH and 
        0 <= center_screen[1] <= MAP_VIEW_HEIGHT and 
        width > 2 and height > 2):
        ellipse_size = max(int(width + 10), int(height + 10))
        if ellipse_size > 2000:
            return
        ellipse_surf = pygame.Surface((ellipse_size, ellipse_size), pygame.SRCALPHA)
        try:
            pygame.draw.ellipse(ellipse_surf, (*color, 60), 
                                (ellipse_size//2 - width//2, ellipse_size//2 - height//2, width, height))
            pygame.draw.ellipse(ellipse_surf, color, 
                                (ellipse_size//2 - width//2, ellipse_size//2 - height//2, width, height), 2)
            if abs(angle) > 1:
                ellipse_surf = pygame.transform.rotate(ellipse_surf, -angle)
            rect = ellipse_surf.get_rect(center=center_screen)
            surface.blit(ellipse_surf, rect)
        except:
            pass

def draw_info_panel(screen, font, small_font, iou, speed, total_scans, pose_uncertainty, amcl_corrections):
    panel_rect = pygame.Rect(MAP_VIEW_WIDTH, 0, INFO_PANEL_WIDTH, screen.get_height())
    pygame.draw.rect(screen, (40, 40, 40), panel_rect)
    pygame.draw.line(screen, (100, 100, 100), (MAP_VIEW_WIDTH, 0), (MAP_VIEW_WIDTH, screen.get_height()), 2)
    
    y_offset = 20
    line_height = 35
    small_line_height = 25
    
    title_text = font.render("Mapping Simulator", True, (255, 255, 255))
    screen.blit(title_text, (MAP_VIEW_WIDTH + 10, y_offset))
    y_offset += line_height + 10
    
    mode_text = small_font.render("(Odometry Only Mode)", True, (255, 255, 100))
    screen.blit(mode_text, (MAP_VIEW_WIDTH + 10, y_offset))
    y_offset += small_line_height + 10
    
    iou_color = (0, 255, 0) if iou > 80 else (255, 255, 0) if iou > 60 else (255, 100, 100)
    iou_text = font.render(f"Mapping IoU: {iou:.1f}%", True, iou_color)
    screen.blit(iou_text, (MAP_VIEW_WIDTH + 10, y_offset))
    y_offset += line_height + 20
    
    if pose_uncertainty:
        pos_error = np.linalg.norm(pose_uncertainty.true_pos - pose_uncertainty.estimated_pos)
        error_color = (0, 255, 0) if pos_error < 20 else (255, 255, 0) if pos_error < 50 else (255, 100, 100)
        pos_error_text = font.render(f"Position Error: {pos_error:.1f} px", True, error_color)
        screen.blit(pos_error_text, (MAP_VIEW_WIDTH + 10, y_offset))
        y_offset += line_height
        uncertainty_value = np.sqrt(np.trace(pose_uncertainty.covariance))
        unc_color = (0, 255, 0) if uncertainty_value < 30 else (255, 255, 0) if uncertainty_value < 70 else (255, 100, 100)
        unc_text = font.render(f"Uncertainty: {uncertainty_value:.1f}", True, unc_color)
        screen.blit(unc_text, (MAP_VIEW_WIDTH + 10, y_offset))
        y_offset += line_height + 10
    
    stats = [
        f"Total Scans: {total_scans:,}",
        f"Robot Speed: {speed} px/frame"
    ]
    if pose_uncertainty:
        true_pos = pose_uncertainty.true_pos.astype(int)
        est_pos = pose_uncertainty.estimated_pos.astype(int)
        stats.extend([
            f"True Position: ({true_pos[0]}, {true_pos[1]})",
            f"Est. Position: ({est_pos[0]}, {est_pos[1]})"
        ])
    for stat in stats:
        stat_text = small_font.render(stat, True, (200, 200, 200))
        screen.blit(stat_text, (MAP_VIEW_WIDTH + 10, y_offset))
        y_offset += small_line_height
        
    y_offset += 20
    controls = [
        "CONTROLS:",
        "↑: Avançar (na direção atual)",
        "↓: Retroceder",
        "←/→: Rotacionar",
        "+ / -: Ajustar Velocidade",
        "R: Reiniciar | S: Salvar Mapa | ESC: Sair",
        "",
        "VISUALIZATION:",
        "  Blue: Estim. LiDAR rays (a partir da pos. estimada)",
        "  Red: True LiDAR rays (a partir da pos. real)",
        "  Green: True Pos",
        "  Yellow: Uncertainty Ellipse",
        "  Gray Dots: Mapped obstacles"
    ]
    for control in controls:
        ctrl_color = (255, 255, 100) if control.endswith(":") else (180, 180, 180)
        ctrl_text = small_font.render(control, True, ctrl_color)
        screen.blit(ctrl_text, (MAP_VIEW_WIDTH + 10, y_offset))
        y_offset += small_line_height

def calculate_mapping_iou(original_map, occupancy_grid, threshold=0.1, epsilon=0.1):
    probs = 1.0 / (1.0 + np.exp(-occupancy_grid))
    mapped_binary = (probs > threshold).astype(np.float32)
    original_binary = np.where(original_map <= 127, 1.0, 0.0)
    mapped_mask = np.abs(occupancy_grid) > epsilon
    if np.sum(mapped_mask) == 0:
        return 0.0
    intersection = np.logical_and(mapped_binary[mapped_mask] == 1, original_binary[mapped_mask] == 1).sum()
    union = np.logical_or(mapped_binary[mapped_mask] == 1, original_binary[mapped_mask] == 1).sum()
    if union == 0:
        return 100.0
    iou = (intersection / union) * 100.0
    return iou

# ---------------------------------------------------------------------------
# Algoritmo de Bresenham para desenhar linhas
# ---------------------------------------------------------------------------
def bresenham(x0, y0, x1, y1):
    points = []
    dx, dy = abs(x1 - x0), abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    err = dx - dy
    while True:
        points.append((x0, y0))
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 > -dy:
            err -= dy
            x0 += sx
        if e2 < dx:
            err += dx
            y0 += sy
    return points

# ---------------------------------------------------------------------------
# Simulação do LiDAR (modos "true" e "estimated")
# ---------------------------------------------------------------------------
def simulate_realistic_lidar(sim_map, robot_pos, mode="true"):
    hits = []
    rays = []
    false_negative_rate = 0.03
    false_positive_rate = 0.01
    max_range_variation = 0.05

    if mode == "true":
        pos = robot_pos.true_pos.astype(int).copy()
        angle = robot_pos.true_angle
        pos[0] = np.clip(pos[0], 0, sim_map.shape[1] - 1)
        pos[1] = np.clip(pos[1], 0, sim_map.shape[0] - 1)
        use_map = True
    elif mode == "estimated":
        sensor_pos = robot_pos.true_pos.astype(int).copy()
        sensor_pos[0] = np.clip(sensor_pos[0], 0, sim_map.shape[1] - 1)
        sensor_pos[1] = np.clip(sensor_pos[1], 0, sim_map.shape[0] - 1)
        sensor_angle = robot_pos.true_angle
        draw_pos = robot_pos.estimated_pos.astype(int)
        draw_angle = robot_pos.estimated_angle
        use_map = True
    else:
        pos = robot_pos.estimated_pos.astype(int)
        angle = robot_pos.estimated_angle
        use_map = False

    for angle_offset in range(-LIDAR_FOV // 2, LIDAR_FOV // 2 + 1, LIDAR_RES):
        if mode == "estimated":
            sensor_ray_angle = math.radians(sensor_angle + angle_offset)
            max_range_actual = LIDAR_RANGE_PX * (1.0 + np.random.uniform(-max_range_variation, max_range_variation))
            max_dist = int(max(1, min(max_range_actual, np.random.normal(max_range_actual, LIDAR_NOISE_STD))))
            x1_sensor = int(sensor_pos[0] + max_dist * math.cos(sensor_ray_angle))
            y1_sensor = int(sensor_pos[1] + max_dist * math.sin(sensor_ray_angle))
            ray_sensor = bresenham(sensor_pos[0], sensor_pos[1], x1_sensor, y1_sensor)
            hit_found = False
            actual_hit = None
            for i, (x, y) in enumerate(ray_sensor):
                if not (0 <= x < sim_map.shape[1] and 0 <= y < sim_map.shape[0]):
                    break
                if sim_map[y, x] <= 127:
                    if np.random.rand() < false_negative_rate:
                        continue
                    noise_x = np.random.normal(0, 1.5)
                    noise_y = np.random.normal(0, 1.5)
                    hit_x = max(0, min(x + int(noise_x), sim_map.shape[1] - 1))
                    hit_y = max(0, min(y + int(noise_y), sim_map.shape[0] - 1))
                    actual_hit = (hit_x, hit_y)
                    hit_found = True
                    break
            if not hit_found:
                if np.random.rand() < false_positive_rate:
                    low_idx = len(ray_sensor) // 2
                    high_idx = min(len(ray_sensor), max_dist // 2)
                    if high_idx > low_idx:
                        false_hit_idx = np.random.randint(low_idx, high_idx)
                        actual_hit = ray_sensor[false_hit_idx]
                else:
                    actual_hit = (x1_sensor, y1_sensor)
            draw_ray_angle = math.radians(draw_angle + angle_offset)
            if actual_hit:
                real_distance = math.sqrt((actual_hit[0] - sensor_pos[0])**2 + (actual_hit[1] - sensor_pos[1])**2)
                draw_hit_x = int(draw_pos[0] + real_distance * math.cos(draw_ray_angle))
                draw_hit_y = int(draw_pos[1] + real_distance * math.sin(draw_ray_angle))
                hits.append((draw_hit_x, draw_hit_y))
                rays.append(((draw_pos[0], draw_pos[1]), (draw_hit_x, draw_hit_y)))
        else:
            ray_angle = math.radians(angle + angle_offset)
            max_range_actual = LIDAR_RANGE_PX * (1.0 + np.random.uniform(-max_range_variation, max_range_variation))
            max_dist = int(max(1, min(max_range_actual, np.random.normal(max_range_actual, LIDAR_NOISE_STD))))
            x1 = int(pos[0] + max_dist * math.cos(ray_angle))
            y1 = int(pos[1] + max_dist * math.sin(ray_angle))
            if not use_map:
                hits.append((x1, y1))
                rays.append(((pos[0], pos[1]), (x1, y1)))
            else:
                ray = bresenham(pos[0], pos[1], x1, y1)
                hit_found = False
                for i, (x, y) in enumerate(ray):
                    if not (0 <= x < sim_map.shape[1] and 0 <= y < sim_map.shape[0]):
                        break
                    if sim_map[y, x] <= 127:
                        if np.random.rand() < false_negative_rate:
                            continue
                        noise_x = np.random.normal(0, 1.5)
                        noise_y = np.random.normal(0, 1.5)
                        hit_x = max(0, min(x + int(noise_x), sim_map.shape[1] - 1))
                        hit_y = max(0, min(y + int(noise_y), sim_map.shape[0] - 1))
                        hits.append((hit_x, hit_y))
                        rays.append(((pos[0], pos[1]), (hit_x, hit_y)))
                        hit_found = True
                        break
                if not hit_found:
                    if np.random.rand() < false_positive_rate:
                        low_idx = len(ray) // 2
                        high_idx = min(len(ray), max_dist // 2)
                        if high_idx > low_idx:
                            false_hit_idx = np.random.randint(low_idx, high_idx)
                            fx, fy = ray[false_hit_idx]
                            hits.append((fx, fy))
                            rays.append(((pos[0], pos[1]), (fx, fy)))
                    else:
                        rays.append(((pos[0], pos[1]), (x1, y1)))
    return hits, rays

# ---------------------------------------------------------------------------
# Função principal da simulação modificada para usar o mapa com padding
# ---------------------------------------------------------------------------
def run_gui_simulator_padded(raw_map):
    # raw_map já possui padding aplicado
    _, sim_map = cv2.threshold(raw_map, 127, 255, cv2.THRESH_BINARY)
    occupancy_grid = np.zeros(sim_map.shape, dtype=np.float32)
    ground_truth = create_ground_truth_reference(raw_map)
    
    pygame.init()
    screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
    pygame.display.set_caption("Mapping Simulator - Odometry Only Mode")
    camera = Camera(sim_map.shape[1], sim_map.shape[0])
    font = pygame.font.Font(None, FONT_SIZE)
    small_font = pygame.font.Font(None, SMALL_FONT_SIZE)
    clock = pygame.time.Clock()
    
    start_pos = find_free_starting_position(sim_map)
    if sim_map[start_pos[1], start_pos[0]] > 127:
        pose_uncertainty = PoseUncertainty([start_pos[0], start_pos[1]])
        sim_started = True
        true_trajectory = [(start_pos[0], start_pos[1])]
        estimated_trajectory = [(start_pos[0], start_pos[1])]
        print(f"Robô automaticamente posicionado em {start_pos}.")
    else:
        pose_uncertainty = None
        sim_started = False
        true_trajectory = []
        estimated_trajectory = []
        print("Clique no mapa para posicionar o robô (área livre).")
    
    estimated_hits = []
    estimated_rays = []
    true_hits = []
    true_rays = []
    
    speed = 3
    total_scans = 0
    amcl_corrections = 0
    last_scan = pygame.time.get_ticks()
    scan_thread = None

    def scan():
        nonlocal estimated_hits, estimated_rays, true_hits, true_rays, last_scan, total_scans
        estimated_hits_new, estimated_rays_new = simulate_realistic_lidar(sim_map, pose_uncertainty, mode="estimated")
        true_hits_new, true_rays_new = simulate_realistic_lidar(sim_map, pose_uncertainty, mode="true")
        update_realistic_grid(occupancy_grid, estimated_hits_new, estimated_rays_new)
        estimated_hits = estimated_hits_new
        estimated_rays = estimated_rays_new
        true_hits = true_hits_new
        true_rays = true_rays_new
        total_scans += 1
        last_scan = pygame.time.get_ticks()

    print("=== Advanced Mapping Simulator - Odometry Only Mode ===")
    print("Use as setas para mover o robô:")
    print("  ↑: Avançar (na direção atual)")
    print("  ↓: Retroceder")
    print("  ←/→: Rotacionar (360°)")
    print("R: Reiniciar | ESC: Sair")
    print("NOTA: A posição REAL é limitada aos limites do mapa enquanto o sensor estimado utiliza o mapa para detectar paredes a partir de sua própria posição.")
    
    running = True
    while running:
        if sim_started and pose_uncertainty:
            camera.update(pose_uncertainty)
        screen.fill((50, 50, 50))
        
        full_map = pygame.Surface((sim_map.shape[1], sim_map.shape[0]))
        full_map.fill((128, 128, 128))
        
        # Converte o occupancy grid para imagem e aplica dilatação
        probs = 1.0 / (1.0 + np.exp(-occupancy_grid))
        grid_img = (255 * (1.0 - probs)).astype(np.uint8)
        kernel = np.ones((BORDER_KERNEL_SIZE, BORDER_KERNEL_SIZE), np.uint8)
        grid_img = cv2.dilate(grid_img, kernel, iterations=1)
        # Obter uma versão colorida para permitir desenho da borda preta
        grid_rgb = np.stack([grid_img] * 3, axis=-1)
        # Usar threshold para gerar uma imagem binária e encontrar os contornos
        ret, binary = cv2.threshold(grid_img, CONTOUR_THRESHOLD, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(binary.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        cv2.drawContours(grid_rgb, contours, -1, (0, 0, 0), thickness=BORDER_THICKNESS)
        grid_surface = pygame.surfarray.make_surface(grid_rgb.swapaxes(0, 1))
        full_map.blit(grid_surface, (0, 0))
        
        if sim_started and pose_uncertainty is not None:
            for start, end in true_rays:
                pygame.draw.line(full_map, (255, 100, 100), start, end, 1)
            for start, end in estimated_rays:
                pygame.draw.line(full_map, (100, 100, 255), start, end, 1)
            for hx, hy in estimated_hits:
                pygame.draw.circle(full_map, (180, 180, 255), (hx, hy), 2)
            if len(true_trajectory) > 1:
                pygame.draw.lines(full_map, (0, 255, 0), False, true_trajectory, 7)
            if len(estimated_trajectory) > 1:
                pygame.draw.lines(full_map, (100, 100, 255), False, estimated_trajectory, 7)
            
            # Desenhar a posição real (verde)
            true_pos = pose_uncertainty.true_pos.astype(int)
            true_pos[0] = np.clip(true_pos[0], 0, sim_map.shape[1] - 1)
            true_pos[1] = np.clip(true_pos[1], 0, sim_map.shape[0] - 1)
            pygame.draw.circle(full_map, (0, 255, 0), true_pos, 6)

            # Desenhar a posição estimada (azul)
            est_pos = pose_uncertainty.estimated_pos.astype(int)
            est_pos[0] = np.clip(est_pos[0], 0, sim_map.shape[1] - 1)
            est_pos[1] = np.clip(est_pos[1], 0, sim_map.shape[0] - 1)
            pygame.draw.circle(full_map, (0, 100, 255), est_pos, 8)

            true_end = (int(true_pos[0] + 20 * math.cos(math.radians(pose_uncertainty.true_angle))),
                        int(true_pos[1] + 20 * math.sin(math.radians(pose_uncertainty.true_angle))))
            pygame.draw.line(full_map, (0, 255, 0), true_pos, true_end, 3)
            
            est_end = (int(est_pos[0] + 20 * math.cos(math.radians(pose_uncertainty.estimated_angle))),
                       int(est_pos[1] + 20 * math.sin(math.radians(pose_uncertainty.estimated_angle))))
            pygame.draw.line(full_map, (0, 100, 255), est_pos, est_end, 4)
            draw_uncertainty_ellipse(full_map, pose_uncertainty, camera)
        
        viewport_rect = pygame.Rect(int(camera.x), int(camera.y), MAP_VIEW_WIDTH, MAP_VIEW_HEIGHT)
        map_width, map_height = sim_map.shape[1], sim_map.shape[0]
        viewport_rect.x = max(0, min(viewport_rect.x, map_width - MAP_VIEW_WIDTH))
        viewport_rect.y = max(0, min(viewport_rect.y, map_height - MAP_VIEW_HEIGHT))
        viewport_rect.width = min(MAP_VIEW_WIDTH, map_width - viewport_rect.x)
        viewport_rect.height = min(MAP_VIEW_HEIGHT, map_height - viewport_rect.y)
        
        map_surface = pygame.Surface((MAP_VIEW_WIDTH, MAP_VIEW_HEIGHT))
        map_surface.blit(full_map, (0, 0), viewport_rect)
        screen.blit(map_surface, (0, 0))
        
        iou = calculate_mapping_iou(ground_truth, occupancy_grid)
        draw_info_panel(screen, font, small_font, iou, speed, total_scans, pose_uncertainty, amcl_corrections)
        
        now = pygame.time.get_ticks()
        if sim_started and pose_uncertainty and now - last_scan > 300 and (scan_thread is None or not scan_thread.is_alive()):
            scan_thread = threading.Thread(target=scan)
            scan_thread.start()

        pygame.display.flip()
        clock.tick(30)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif event.key == pygame.K_r:
                    if sim_started:
                        save_final_map(occupancy_grid)
                    run_gui_simulator_padded(raw_map)
                    return
                elif sim_started and pose_uncertainty:
                    dx, dy, dtheta = 0, 0, 0
                    angle_rad = math.radians(pose_uncertainty.estimated_angle)
                    if event.key == pygame.K_UP:
                        dx = speed * math.cos(angle_rad)
                        dy = speed * math.sin(angle_rad)
                    elif event.key == pygame.K_DOWN:
                        dx = -speed * math.cos(angle_rad)
                        dy = -speed * math.sin(angle_rad)
                    elif event.key == pygame.K_LEFT:
                        dtheta = -10
                    elif event.key == pygame.K_RIGHT:
                        dtheta = 10
                    elif event.key == pygame.K_PLUS or event.key == pygame.K_EQUALS:
                        speed = min(speed + 1, 20)
                    elif event.key == pygame.K_MINUS:
                        speed = max(speed - 1, 1)
                    elif event.key == pygame.K_s:
                        if sim_started:
                            save_final_map(occupancy_grid)
                            print("Mapa salvo! Pressione 'S' novamente para salvar outro.")
                    if dx != 0 or dy != 0 or dtheta != 0:
                        pose_uncertainty.update_pose(dx, dy, dtheta, sim_map)
                        true_trajectory.append(tuple(pose_uncertainty.true_pos.astype(int)))
                        estimated_trajectory.append(tuple(pose_uncertainty.estimated_pos.astype(int)))
    if sim_started:
        save_final_map(occupancy_grid)
    
    pygame.quit()

# ---------------------------------------------------------------------------
# Execução Principal
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python mapping_simulator.py caminho/para/mapa.png")
        sys.exit(1)
    map_file = sys.argv[1]
    if not os.path.exists(map_file):
        print(f"Erro: Arquivo '{map_file}' não encontrado.")
        sys.exit(1)
    # Carrega o mapa com padding para garantir área extra
    padded_map = load_and_pad_map(map_file, pad=150)
    run_gui_simulator_padded(padded_map)

