from __future__ import division

import ode
from OpenGL.GL import *

from geometry import *

### This is the common library connecting orbit-edit.py (which depends on the Blender API) and the rest of Orbit Ribbon

class Mesh:
	"""A mesh (collection of vertices and faces w/ normals) exported from the 3D editor.
	
	Data attributes:
	vertices - A list of vertices as Points.
	normals - A list of vertex normal vectors as Points.
	faces - A list of faces as ((vi, vi, vi), (n, n, n)), where vi is an index into vertices, and ni is an index into normals.
	material - A string describing the name of the Material for this Mesh, or None if there is no Material.
	"""
	def __init__(self, vertices, normals, faces, material):
		self.vertices = vertices
		self.normals = normals
		self.faces = faces
		self.material = material
		self._trimesh_data = None
		self._pkg_parent = None
	
	def trimesh_data(self):
		"""Returns an ode.TriMeshData for this mesh. This is cached after the first calculation."""
		if self._trimesh_data is None:
			self._trimesh_data = ode.TriMeshData()
			tdat.build(self.vertices, [f[0] for f in self.faces])
		return self._trimesh_data
	
	def draw_gl(self):
		"""Draws the object using OpenGL commands. This is suitable for calling within display list initialization."""
		if self.material is not None:
			self._pkg_parent.materials[self.material]._draw_gl()
		glBegin(GL_TRIANGLES)
		for verts, normals in self._faces:
			for i in range(3):
				glNormal3fv(normals[i])
				glVertex3fv(verts[i])
		glEnd()


class Material:
	"""A material (texture and appearance) exported from the 3D editor.
	
	Data attributes:
	amb_col, dif_col, spe_col - The ambient, diffuse, and specular colors, each as 4-tuples.
	"""
	def __init__(self, amb_col, dif_col, spe_col):
		self.amb_col = amb_col
		self.dif_col = dif_col
		self.spe_col = spe_col
		self._pkg_parent = None
	
	def _draw_gl(self):
		# FIXME: If I'm going to use this, I need to not enable GL_COLOR_MATERIAL
		glMaterialfv(GL_FRONT, GL_AMBIENT, self.amb_col)
		glMaterialfv(GL_FRONT, GL_DIFFUSE, self.dif_col)
		glMaterialfv(GL_FRONT, GL_SPECULAR, self.spe_col)
		

class Area:
	"""A description of an in-game location, exported from the 3D editor.
	
	Data attributes:
	objects - A list of area objects as (objname, meshname, position, rotation).
	"""
	def __init__(self, objects):
		self.objects = objects
		self._pkg_parent = None


class Mission:
	"""A description of an in-game mission, which takes place in an Area but may add more objects.
	
	Data attributes:
	area_name - The Area that this mission takes place in.
	objects - A list of mission-specific objects as (objname, meshname, position, rotation).
	"""
	def __init__(self, area_name, objects):
		self.area_name = area_name
		self.objects = objects
		self._pkg_parent = None


class ExportPackage:
	"""An object containing named instances of other editorexport objects.
	
	This will be the object within pickle files generated by orbit-edit.py
	
	Data attributes:
	meshes - A dictionary of named Mesh objects.
	materials - A dictionary of named Material objects.
	areas - A dictionary of named Area objects.
	missions - A dictionary of named Mission objects.
	"""
	def __init__(self, meshes, materials, areas, missions):
		self.meshes = meshes
		self.materials = materials
		self.areas = areas
		self.missions = missions
		
		# Make sure everything can get at anything else by name
		for e in self.meshes, self.materials, self.areas, self.missions:
			for x in e:
				x._pkg_parent = self
