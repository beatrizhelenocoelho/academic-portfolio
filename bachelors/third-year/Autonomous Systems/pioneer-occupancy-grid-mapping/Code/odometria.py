import rosbag
import matplotlib.pyplot as plt

bag = rosbag.Bag("11.bag")

x_vals, y_vals = [], []

for topic, msg, t in bag.read_messages(topics=['/pose']):
    # Acesso corrigido para PoseWithCovarianceStamped
    x_vals.append(msg.pose.pose.position.x)
    y_vals.append(msg.pose.pose.position.y)

bag.close()

plt.plot(x_vals, y_vals, marker='o', label="Trajetória Odométrica")
plt.title("Trajetória estimada (via /pose)")
plt.xlabel("X (m)")
plt.ylabel("Y (m)")
plt.axis("equal")
plt.grid(True)
plt.legend()
plt.show()