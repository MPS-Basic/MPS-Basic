import numpy as np
import meshio
from pathlib import Path


class Box2d:
    def __init__(self, x_min, x_max, y_min, y_max):
        self.senter = np.array([(x_min + x_max) / 2, (y_min + y_max) / 2])
        self.size = np.array([x_max - x_min, y_max - y_min])

    def sdf(self, position):
        q = np.abs(position - self.senter) - self.size / 2
        return np.linalg.norm(np.maximum(q, 0)) + np.min(np.maximum(q, 0))


def generate(l0, prof_path, vtu_path):
    fluid_domain = Box2d(-0.18, 0.18, 0, 0.48)
    empty_domain = Box2d(-0.18, 0.18, -0.48, 0.6)
    wall_domain = Box2d(-0.18 - 2 * l0, 0.18 + 2 * l0, -2 * l0, 0.6)
    dummy_domain = Box2d(-0.18 - 4 * l0, 0.18 + 4 * l0, -4 * l0, 0.6)

    positions = []
    types = []

    for ix in range(round(-0.18 - 4 * l0), round(0.18 / l0) + 4):
        for iy in range(round(-4 * l0), round(0.6 / l0) + 4):
            pos_i = np.array([ix * l0, iy * l0])

            if empty_domain.sdf(pos_i) < 0:
                continue
            elif fluid_domain.sdf(pos_i) < 0:
                positions.append(pos_i)
                types.append(0)
            elif wall_domain.sdf(pos_i) < 0:
                positions.append(pos_i)
                types.append(1)
            elif dummy_domain.sdf(pos_i) < 0:
                positions.append(pos_i)
                types.append(2)

    # write prof file
    # write vtu file with meshio


if __name__ == "__main__":
    l0 = 0.012
    project_dir = Path(__file__).parent.parent
    dst_dit = project_dir / "input" / "hydrostatic"
