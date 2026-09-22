from __future__ import absolute_import
from ctypes import pointer
from functools import partial
from .main import bass_call, update_3d_system
from .external.pybass import *


def _getter(base_prop, attr, obj):
    """

    Args:
      base_prop: 
      attr: 
      obj: 

    Returns:

    """
    return getattr(getattr(obj, base_prop), attr)


def _setter(base_prop, subattr, obj, val):
    """

    Args:
      base_prop: 
      subattr: 
      obj: 
      val: 

    Returns:

    """
    old = getattr(obj, base_prop)
    setattr(old, subattr, val)
    setattr(obj, base_prop, old)


class Listener(object):
    """ """
    def get_3d_position(self):
        """ """
        res = {
            "position": BASS_3DVECTOR(),
            "velocity": BASS_3DVECTOR(),
            "front": BASS_3DVECTOR(),
            "top": BASS_3DVECTOR(),
        }
        bass_call(
            BASS_Get3DPosition,
            pointer(res["position"]),
            pointer(res["velocity"]),
            pointer(res["front"]),
            pointer(res["top"]),
        )
        return res

    @update_3d_system
    def set_3d_position(self, position=None, velocity=None, front=None, top=None):
        """Sets the position, velocity, and orientation of the listener (ie. the player).

        Args:
          position:  (Default value = None)
          velocity:  (Default value = None)
          front:  (Default value = None)
          top:  (Default value = None)

        Returns:

        """
        old = self.get_3d_position()
        if position is None:
            position = old["position"]
        if velocity is None:
            velocity = old["velocity"]
        if front is None:
            front = old["front"]
        if top is None:
            top = old["top"]
        position = pointer(position)
        velocity = pointer(velocity)
        front = pointer(front)
        top = pointer(top)
        bass_call(BASS_Set3DPosition, position, velocity, front, top)

    def get_position(self):
        """ """
        return self.get_3d_position()["position"]

    def set_position(self, position):
        """

        Args:
          position: 

        Returns:

        """
        self.set_3d_position(position=position)

    position = property(fget=get_position, fset=set_position)

    x = property(
        fget=partial(_getter, "position", "x"), fset=partial(_setter, "position", "x")
    )

    y = property(
        fget=partial(_getter, "position", "y"), fset=partial(_setter, "position", "y")
    )

    z = property(
        fget=partial(_getter, "position", "z"), fset=partial(_setter, "position", "z")
    )

    def get_velocity(self):
        """ """
        return self.get_3d_position()["velocity"]

    def set_velocity(self, velocity):
        """

        Args:
          velocity: 

        Returns:

        """
        self.set_3d_position(velocity=velocity)

    velocity = property(fget=get_velocity, fset=set_velocity)

    x_velocity = property(
        fget=partial(_getter, "velocity", "x"), fset=partial(_setter, "velocity", "x")
    )

    y_velocity = property(
        fget=partial(_getter, "velocity", "y"), fset=partial(_setter, "velocity", "y")
    )

    z_velocity = property(
        fget=partial(_getter, "velocity", "z"), fset=partial(_setter, "velocity", "z")
    )

    def get_front(self):
        """ """
        return self.get_3d_position()["front"]

    def set_front(self, front):
        """

        Args:
          front: 

        Returns:

        """
        self.set_3d_position(front=front)

    front = property(fget=get_front, fset=set_front)

    front_x = property(
        fget=partial(_getter, "front", "x"), fset=partial(_setter, "front", "x")
    )

    front_y = property(
        fget=partial(_getter, "front", "y"), fset=partial(_setter, "front", "y")
    )

    front_z = property(
        fget=partial(_getter, "front", "z"), fset=partial(_setter, "front", "z")
    )

    def get_top(self):
        """ """
        return self.get_3d_position()["top"]

    def set_top(self, top):
        """

        Args:
          top: 

        Returns:

        """
        self.set_3d_position(top=top)

    top = property(fget=get_top, fset=set_top)

    top_x = property(
        fget=partial(_getter, "front", "x"), fset=partial(_setter, "front", "x")
    )

    top_y = property(
        fget=partial(_getter, "front", "y"), fset=partial(_setter, "front", "y")
    )

    top_z = property(
        fget=partial(_getter, "front", "z"), fset=partial(_setter, "front", "z")
    )
