extends CharacterBody3D

const SPEED := 5.2
const JUMP_VELOCITY := 5.5
const LOOK_SENSITIVITY := 0.003
var camera: Camera3D
var pitch := 0.0

func _ready() -> void:
	camera = get_node("Camera3D")
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		_rotate_view(event.relative)
	elif event is InputEventScreenDrag:
		_rotate_view(event.relative)
	elif event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
		Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
	elif event is InputEventMouseButton and event.pressed:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _rotate_view(relative: Vector2) -> void:
	rotate_y(-relative.x * LOOK_SENSITIVITY)
	pitch = clampf(pitch - relative.y * LOOK_SENSITIVITY, -1.45, 1.45)
	camera.rotation.x = pitch

func _physics_process(delta: float) -> void:
	if not is_on_floor():
		velocity.y -= 18.0 * delta
	if Input.is_action_just_pressed("jump") and is_on_floor():
		velocity.y = JUMP_VELOCITY
	var input_dir := Input.get_vector("move_left","move_right","move_forward","move_back")
	var direction := (transform.basis * Vector3(input_dir.x,0,input_dir.y)).normalized()
	if direction:
		velocity.x = direction.x * SPEED
		velocity.z = direction.z * SPEED
	else:
		velocity.x = move_toward(velocity.x,0,SPEED * delta * 8)
		velocity.z = move_toward(velocity.z,0,SPEED * delta * 8)
	move_and_slide()
