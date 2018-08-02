from . import asset_manager
from taichi.core import tc_core
from taichi.misc.util import P, array2d_to_ndarray

import random

class Texture:

  def __init__(self, name, **kwargs):
    if isinstance(name, str):
      self.c = tc_core.create_texture(name)
      kwargs = asset_manager.asset_ptr_to_id(kwargs)
      self.c.initialize(P(**kwargs))
    else:
      self.c = name
    self.id = tc_core.register_texture(self.c)

  @staticmethod
  def wrap_texture(value):
    if isinstance(value, tuple):
      value = tuple(list(value) + [0] * (4 - len(value)))
      return Texture('const', value=value)
    elif isinstance(value, float) or isinstance(value, int):
      return Texture('const', value=(value, value, value, value))
    else:
      return value

  def __int__(self):
    return self.id

  def __mul__(self, other):
    other = self.wrap_texture(other)
    return Texture("mul", tex1=self, tex2=other)

  def __rmul__(self, other):
    return self * other

  def __add__(self, other):
    other = self.wrap_texture(other)
    return Texture("linear_op", alpha=1, tex1=self, beta=1, tex2=other)

  def __radd__(self, other):
    return self.__add__(other)

  def __sub__(self, other):
    other = self.wrap_texture(other)
    return Texture("linear_op", alpha=1, tex1=self, beta=-1, tex2=other)

  def __rsub__(self, other):
    other = self.wrap_texture(other)
    return Texture("linear_op", alpha=-1, tex1=self, beta=1, tex2=other)

  def clamp(self):
    return Texture(
        "linear_op", alpha=1, tex1=self, beta=0, tex2=self, need_clamp=True)

  def flip(self, flip_axis):
    return Texture("flip", tex=self, flip_axis=flip_axis)

  def zoom(self, zoom=(2, 2, 2), center=(0, 0, 0), repeat=True):
    return Texture("zoom", tex=self, center=center, zoom=zoom, repeat=repeat)

  def perlin_noise(self, bounds=(0, 1), scale=(10, 10, 10)):
    rnd_offset = (random.random(), random.random(), random.random())
    return self * ((Texture('perlin').zoom(scale, rnd_offset) + 1) * 0.5 * (bounds[1] - bounds[0]) + bounds[0])

  def repeat(self, repeat_x, repeat_y, repeat_z):
    return Texture(
        "repeat",
        tex=self,
        repeat_u=repeat_x,
        repeat_v=repeat_y,
        repeat_w=repeat_z)

  def rotate(self, times):
    return Texture("rotate", tex=self, times=times)

  def translate(self, translation):
    return Texture("trans", tex=self, translation=translation)

  def fract(self):
    return Texture("fract", tex=self)

  def rasterize(self, resolution_x=256, resolution_y=-1):
    if resolution_y == -1:
      resolution_y = resolution_x
    return Texture(
        "rasterize",
        tex=self,
        resolution_x=resolution_x,
        resolution_y=resolution_y)

  def rasterize_to_ndarray(self, res=(512, 512)):
    array2d = self.c.rasterize(res[0], res[1])
    return array2d_to_ndarray(array2d)

  def show(self,
           title='Taichi Texture Renderer',
           res=(512, 512),
           post_processor=None):
    from taichi.gui.image_viewer import show_image
    img = self.rasterize_to_ndarray(res)
    if post_processor:
      img = post_processor.process(img)
    show_image(title, img)

  @staticmethod
  def create_taichi_wallpaper(n=20, scale=0.96, rotation=0):
    taichi = Texture('taichi', scale=scale, rotation=rotation)
    rep = Texture("repeat", repeat_u=n, repeat_v=n, tex=taichi)
    rep = Texture(
        "checkerboard", tex1=rep, tex2=0 * rep, repeat_u=n,
        repeat_v=n) * 0.8 + 0.1
    return rep.clamp().flip(1)

  @staticmethod
  def from_render_particles(resolution, particles):
    return Texture(
        tc_core.rasterize_render_particles(P(resolution=resolution), particles))
