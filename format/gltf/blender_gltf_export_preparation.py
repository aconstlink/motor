import bpy

def clear_camera_shots():
    collection = bpy.data.collections.get("motor_sequence")
    if collection:
        for obj in collection.objects:
            bpy.data.objects.remove( obj, do_unlink=True)

        bpy.data.collections.remove( collection )
        collection = None
    

            
def create_camera_shots():
    scene = bpy.context.scene
    fps = scene.render.fps / scene.render.fps_base

    markers = sorted(
        [m for m in scene.timeline_markers if m.camera is not None],
        key=lambda m: m.frame
    )

    collection = bpy.data.collections.get("motor_sequence")
    
    if collection is None:
        collection = bpy.data.collections.new("motor_sequence")
        scene.collection.children.link(collection)

    for i, marker in enumerate(markers):
        start_frame = marker.frame
        end_frame = markers[i + 1].frame if i + 1 < len(markers) else scene.frame_end

        start_sec = (start_frame - scene.frame_start) / fps
        end_sec = (end_frame - scene.frame_start) / fps

        cam = marker.camera

        empty = bpy.data.objects.new(f"shot.{i:03d}.{cam.name}", None)
        #empty.empty_display_type = 'SPHERE'
        #empty.empty_display_size = 0.5

        collection.objects.link(empty)
        
        empty.parent = cam
        empty.matrix_parent_inverse.identity()
        empty.matrix_local.identity()

        empty["motor.type"] = "camera_shot"
        empty["motor.seq_start"] = start_sec
        empty["motor.seq_end"] = end_sec
        empty["motor.seq_order"] = i
        empty["motor.camera"] = cam.name

clear_camera_shots() 
create_camera_shots() 