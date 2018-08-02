from taichi.misc.util import *
from taichi.two_d import *
from taichi import get_asset_path

if __name__ == '__main__':
  scale = 8
  res = (80 * scale, 40 * scale)
  simulator = MPMSimulator(
      res=res, simulation_time=30, frame_dt=6e-2, base_delta_t=1e-3)

  simulator.add_event(-1,
                      lambda s: s.add_particles_texture(Vector(1, 0.60), 1.8, get_asset_path('textures/hybrid.png'),
                                                        'ep', theta_c=1, theta_s=1))

  levelset = simulator.create_levelset()
  levelset.add_polygon([(0.05, 0.05), (1, 0.4), (1.95, 0.05), (1.95, 0.95),
                        (0.05, 0.95)], True)
  levelset.set_friction(0)
  simulator.set_levelset(levelset)
  window = SimulationWindow(
      1280,
      simulator,
      color_schemes['bw'],
      levelset_supersampling=2,
      show_images=True)
