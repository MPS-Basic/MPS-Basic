import sys
import meshio
import matplotlib.pyplot as plt

# Read VTK file
mesh = meshio.read(sys.argv[1])
x = mesh.points[:, 0]
y = mesh.points[:, 1]
p  = mesh.point_data['Pressure'][:, 0]
t = mesh.field_data['Time'][0, 0]

fig = plt.figure(figsize=(6, 6))
ax = fig.add_subplot(111)
ax.set_aspect('equal', 'box')
ax.set_title('Pressure at t = {:.2f}'.format(t))
aximg =  ax.scatter(x, y, c=p, cmap='viridis', s=24)

fig.colorbar(aximg)
plt.savefig(sys.argv[2])