extends Node3D

const WORLD_SIZE := 24
const BLOCK_SIZE := 1.0
var sun: DirectionalLight3D
var time_of_day := 0.28
var player: CharacterBody3D

const DIRS := [Vector3i(1,0,0),Vector3i(-1,0,0),Vector3i(0,1,0),Vector3i(0,-1,0),Vector3i(0,0,1),Vector3i(0,0,-1)]
const FACE_VERTS := [
	[Vector3(0,0,0),Vector3(0,1,0),Vector3(0,1,1),Vector3(0,0,1)],
	[Vector3(1,0,1),Vector3(1,1,1),Vector3(1,1,0),Vector3(1,0,0)],
	[Vector3(0,1,1),Vector3(0,1,0),Vector3(1,1,0),Vector3(1,1,1)],
	[Vector3(0,0,0),Vector3(0,0,1),Vector3(1,0,1),Vector3(1,0,0)],
	[Vector3(1,0,1),Vector3(0,0,1),Vector3(0,1,1),Vector3(1,1,1)],
	[Vector3(0,0,0),Vector3(1,0,0),Vector3(1,1,0),Vector3(0,1,0)]
]
const NORMALS := [Vector3.LEFT,Vector3.RIGHT,Vector3.UP,Vector3.DOWN,Vector3.BACK,Vector3.FORWARD]

func _ready() -> void:
	_build_world()
	_build_lighting()
	_build_player()
	_build_hud()

func _block_height(x: int, z: int) -> int:
	return clampi(4 + int(round(sin(float(x)*0.31)*1.8 + cos(float(z)*0.27)*1.5)), 2, 8)

func _build_world() -> void:
	var blocks := {}
	for x in range(WORLD_SIZE):
		for z in range(WORLD_SIZE):
			var h := _block_height(x,z)
			for y in range(h+1):
				blocks[Vector3i(x,y,z)] = 0 if y == h else (2 if y < h-3 else 1)
	var st := SurfaceTool.new()
	st.begin(Mesh.PRIMITIVE_TRIANGLES)
	var mat := StandardMaterial3D.new()
	mat.vertex_color_use_as_albedo = true
	st.set_material(mat)
	var colors := [Color(0.25,0.62,0.22),Color(0.42,0.29,0.17),Color(0.43,0.44,0.46)]
	for cell in blocks:
		var kind: int = blocks[cell]
		for face in range(6):
			if not blocks.has(cell + DIRS[face]):
				var base := Vector3(cell) * BLOCK_SIZE
				for tri in [0,1,2,0,2,3]:
					st.set_normal(NORMALS[face])
					st.set_color(colors[kind])
					st.add_vertex(base + FACE_VERTS[face][tri] * BLOCK_SIZE)
	var mesh := st.commit()
	var body := StaticBody3D.new()
	body.name = "VoxelTerrain"
	var visual := MeshInstance3D.new()
	visual.mesh = mesh
	body.add_child(visual)
	var collision := CollisionShape3D.new()
	collision.shape = mesh.create_trimesh_shape()
	body.add_child(collision)
	add_child(body)

func _build_lighting() -> void:
	var env := WorldEnvironment.new()
	var environment := Environment.new()
	environment.background_mode = Environment.BG_SKY
	environment.sky = Sky.new()
	environment.sky.sky_material = ProceduralSkyMaterial.new()
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	environment.ambient_light_color = Color(0.62,0.68,0.8)
	environment.ambient_light_energy = 0.7
	env.environment = environment
	add_child(env)
	sun = DirectionalLight3D.new()
	sun.shadow_enabled = true
	add_child(sun)

func _build_player() -> void:
	player = CharacterBody3D.new()
	player.position = Vector3(WORLD_SIZE/2.0, 13, WORLD_SIZE/2.0)
	player.name = "Player"
	var shape_node := CollisionShape3D.new()
	var capsule := CapsuleShape3D.new()
	capsule.radius = 0.34
	capsule.height = 1.8
	shape_node.shape = capsule
	player.add_child(shape_node)
	var camera := Camera3D.new()
	camera.name = "Camera3D"
	camera.position.y = 0.62
	camera.current = true
	player.add_child(camera)
	player.set_script(load("res://Scripts/player.gd"))
	add_child(player)

func _build_hud() -> void:
	var layer := CanvasLayer.new()
	add_child(layer)
	var title := Label.new()
	title.position = Vector2(18,18)
	title.add_theme_font_size_override("font_size",22)
	title.text = "MineReal  |  Sobrevivência"
	layer.add_child(title)
	var hint := Label.new()
	hint.position = Vector2(18,52)
	hint.text = "WASD: mover  •  Espaço: pular  •  Mouse/toque: olhar"
	layer.add_child(hint)

func _process(delta: float) -> void:
	time_of_day = fposmod(time_of_day + delta*0.003,1.0)
	sun.light_energy = maxf(0.12,sin(time_of_day*TAU)*0.5+0.5)
	sun.rotation_degrees.x = -90.0 + time_of_day*360.0
