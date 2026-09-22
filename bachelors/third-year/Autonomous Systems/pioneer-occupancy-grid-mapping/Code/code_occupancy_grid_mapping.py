#!/usr/bin/env python3
import rospy
import cv2
import yaml
import numpy as np
from sensor_msgs.msg import LaserScan
from nav_msgs.msg import OccupancyGrid
from std_msgs.msg import Header
from tf.transformations import euler_from_quaternion
import math
import matplotlib.pyplot as plt

import tf2_ros

# parâmetros do mapa
MAP_WIDTH = 50  # metros
MAP_HEIGHT = 50
RESOLUTION = 0.05  # 5 cm por célula
MAP_SIZE_X = int(MAP_WIDTH / RESOLUTION)
MAP_SIZE_Y = int(MAP_HEIGHT / RESOLUTION)
log_odds = np.zeros((MAP_SIZE_X, MAP_SIZE_Y))  # mapa em log-odds

# posição atual do robô (global)
pose = {'x': 0.0, 'y': 0.0, 'theta': 0.0}



def save_map_and_yaml(data, width, height, resolution, filename='map.pgm', yaml_filename='map.yaml'):
    arr = np.array(data).reshape((height, width))
    img = np.zeros((height, width), dtype=np.uint8)
    img[arr == 0] = 255
    img[arr == 100] = 0
    img[arr == -1] = 205  # para ficar igual ao do ros 

    # mascara para detectar zona mapeada
    mask = (img != 205).astype(np.uint8)

    if cv2.countNonZero(mask) == 0:
        print("Mapa vazio")
        cv2.imwrite(filename, img)
        return

    # cortar à volta da zona útil - para o mapa nao ficar muito pequeno
    x, y, w, h = cv2.boundingRect(mask)
    img_cropped = img[y:y+h, x:x+w]
    cv2.imwrite(filename, img_cropped)

    # nova origem, em metros
    origin_x = -MAP_WIDTH / 2.0 + x * resolution
    origin_y = -MAP_HEIGHT / 2.0 + (height - (y + h)) * resolution  # y invertido

    origin = [float(f"{origin_x:.4f}"), float(f"{origin_y:.4f}"), 0.0]

    # yaml
    map_metadata = {
        'image': filename,
        'resolution': resolution,
        'origin': origin,
        'negate': 0,
        'occupied_thresh': 0.65,
        'free_thresh': 0.196
    }

    with open(yaml_filename, 'w') as f:
        yaml.dump(map_metadata, f)

    print(f"Map salvo em {filename}")
    print(f"YAML salvo em {yaml_filename} com origem: {origin}")

def get_pose_from_tf(tf_buffer):
    try:
        trans = tf_buffer.lookup_transform('odom', 'base_link', rospy.Time(0), rospy.Duration(1.0))
        x = trans.transform.translation.x
        y = trans.transform.translation.y
        q = trans.transform.rotation
        _, _, yaw = euler_from_quaternion([q.x, q.y, q.z, q.w])
        return {'x': x, 'y': y, 'theta': yaw}
    except (tf2_ros.LookupException, tf2_ros.ConnectivityException, tf2_ros.ExtrapolationException):
        rospy.logwarn("TF transform not available")
        return None

MIN_VALID_RANGE = 0.02  # 2 cm
MAX_VALID_RANGE = 4.0   # 4 m

def laser_callback(msg):
    global log_odds, pose
    try:
        if msg.header.stamp == rospy.Time(0):
            rospy.logwarn("Timestamp inválido no LaserScan")
            return
        

        angle = msg.angle_min

        for  r in msg.ranges:
            if np.isinf(r) or np.isnan(r) or r < MIN_VALID_RANGE or r > MAX_VALID_RANGE:
                angle += msg.angle_increment
                continue

            x_end = pose['x'] + r * math.cos(angle + pose['theta'])
            y_end = pose['y'] + r * math.sin(angle + pose['theta'])

            x0 = int((pose['x'] + MAP_WIDTH / 2.0) / RESOLUTION)
            y0 = int((pose['y'] + MAP_HEIGHT / 2.0) / RESOLUTION)
            x1 = int((x_end + MAP_WIDTH / 2.0) / RESOLUTION)
            y1 = int((y_end + MAP_HEIGHT / 2.0) / RESOLUTION)

            for x, y in bresenham(x0, y0, x1, y1):
                if 0 <= x < MAP_SIZE_X and 0 <= y < MAP_SIZE_Y:
                    log_odds[x][y] -= 0.4
                    log_odds[x][y] = max(min(log_odds[x][y], 10), -10)

            if 0 <= x1 < MAP_SIZE_X and 0 <= y1 < MAP_SIZE_Y:
                log_odds[x1][y1] += 0.85
                log_odds[x1][y1] = max(min(log_odds[x1][y1], 10), -10)

            angle += msg.angle_increment

    except Exception as e:
        rospy.logwarn(f"Erro no callback do laser: {e}")


def bresenham(x0, y0, x1, y1):
    points = []
    dx = abs(x1 - x0)
    dy = abs(y1 - y0)
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

def publish_map(pub):
    grid = OccupancyGrid()
    grid.header = Header()
    grid.header.stamp = rospy.Time.now()
    #grid.header.frame_id = "map"
    grid.header.frame_id = "odom"

    grid.info.resolution = RESOLUTION
    grid.info.width = MAP_SIZE_X
    grid.info.height = MAP_SIZE_Y
    grid.info.origin.position.x = -MAP_WIDTH / 2.0
    grid.info.origin.position.y = -MAP_HEIGHT / 2.0
    grid.info.origin.position.z = 0.0
    grid.info.origin.orientation.w = 1.0

    data = []
    for y in range(MAP_SIZE_Y):
        for x in range(MAP_SIZE_X):
            l = max(min(log_odds[x][y], 10), -10)
            p = 1 - 1 / (1 + math.exp(l))
            if p < 0.01:
                data.append(0)
            elif p > 0.97:
                data.append(100)
            else:
                data.append(-1)

    grid.data = data
    pub.publish(grid)

    return data  # retorna os dados do mapa

def main():
    rospy.init_node('occupancy_grid_mapper')

    tf_buffer = tf2_ros.Buffer()
    tf_listener = tf2_ros.TransformListener(tf_buffer)

    rospy.Subscriber('/scan', LaserScan, laser_callback)
    map_pub = rospy.Publisher('/my_map', OccupancyGrid, queue_size=1)

    rate = rospy.Rate(10)  # 10 Hz

    global pose
    last_save_time = 0
    save_interval = 10  # segundos

    while not rospy.is_shutdown():

        pose_tf = get_pose_from_tf(tf_buffer)

        if pose_tf is None:
            rate.sleep()
            continue  # ignorar esta iteração se não houver TF

        if pose_tf is not None:
            pose = pose_tf

        grid_data = publish_map(map_pub)

        now = rospy.get_time()
        if now - last_save_time > save_interval:
            save_map_and_yaml(grid_data, MAP_SIZE_X, MAP_SIZE_Y, RESOLUTION)
            last_save_time = now

        rate.sleep()

if __name__ == '__main__':
    main()