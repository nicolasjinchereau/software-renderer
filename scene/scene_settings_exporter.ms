fn saveSceneInfo=(
	local layout = stringStream ""

	format "{\n" to:layout
	allObjects = $*
	i = 0
	for obj in allObjects do(
		
		if isKindOf obj Targetobject then
			continue;
		
		if i > 0 then
			format ",\n" to:layout
		
		i = i + 1
		
		format "\t\"%\": {\n" obj.name to:layout
		
		if isKindOf obj TargetDirectionalLight then
		(
			pos = [obj.pos.x * 0.01, obj.pos.z * 0.01, obj.pos.y * 0.01]
			dir = normalize(obj.target.pos - obj.pos)
			dir = [dir.x, dir.z, dir.y]
			
			col = obj.rgb / 255.0
			format "\t\t\"color\": [%, %, %, 1.0],\n" col.r col.g col.b to:layout
			format "\t\t\"multiplier\": %,\n" obj.Multiplier to:layout
			format "\t\t\"pos\": [%, %, %],\n" pos.x pos.y pos.z to:layout
			format "\t\t\"dir\": [%, %, %]\n" dir.x dir.y dir.z to:layout
		)
		else if isKindOf obj Omnilight then
		(
			pos = [obj.pos.x * 0.01, obj.pos.z * 0.01, obj.pos.y * 0.01]
			col = obj.rgb / 255.0
			format "\t\t\"color\": [%, %, %, 1.0],\n" col.r col.g col.b to:layout
			format "\t\t\"multiplier\": %,\n" obj.Multiplier to:layout
			format "\t\t\"pos\": [%, %, %],\n" pos.x pos.y pos.z to:layout
			format "\t\t\"distAttenMin\": %,\n" (obj.farAttenStart * 0.01) to:layout
			format "\t\t\"distAttenMax\": %\n" (obj.farAttenEnd * 0.01) to:layout
		)
		else if isKindOf obj targetSpot then
		(
			pos = [obj.pos.x * 0.01, obj.pos.z * 0.01, obj.pos.y * 0.01]
			dir = normalize(obj.target.pos - obj.pos)
			dir = [dir.x, dir.z, dir.y]
			
			col = obj.rgb / 255.0
			format "\t\t\"color\": [%, %, %, 1.0],\n" col.r col.g col.b to:layout
			format "\t\t\"multiplier\": %,\n" obj.Multiplier to:layout
			format "\t\t\"pos\": [%, %, %],\n" pos.x pos.y pos.z to:layout
			format "\t\t\"dir\": [%, %, %],\n" dir.x dir.y dir.z to:layout
			format "\t\t\"distAttenMin\": %,\n" (obj.farAttenStart * 0.01) to:layout
			format "\t\t\"distAttenMax\": %,\n" (obj.farAttenEnd * 0.01) to:layout
			format "\t\t\"angAttenMin\": %,\n" obj.hotspot to:layout
			format "\t\t\"angAttenMax\": %\n" obj.falloff to:layout
		)
		else
		(
			pos = [obj.pos.x * 0.01, obj.pos.z * 0.01, obj.pos.y * 0.01]
			rot = [obj.rotation.x_rotation, -obj.rotation.z_rotation, obj.rotation.y_rotation]
			format "\t\t\"pos\": [%, %, %],\n" pos.x pos.y pos.z to:layout
			format "\t\t\"rot\": [%, %, %]\n" rot.x rot.y rot.z to:layout
		)
		
		format "\t}" to:layout
	)
	
	col = ambientColor / 255.0
	format ",\n" to:layout
	format "\t\"ambient_light\": {\n" to:layout
	format "\t\t\"color\": [%, %, %, 1.0],\n" col.r col.g col.b to:layout
	format "\t\t\"multiplier\": %\n" 1.0 to:layout
	format "\t}" to:layout
	
	format "\n}\n" to:layout
	
	filename = getSaveFileName caption:"Save Scene Info to JSON" \
	filename:"scene_settings.json" \
	types:"JSON File (*.json)|*.json" \
	historyCategory:"JSON Scene Info"

	if filename != undefined then
	(
		file = (createFile filename)
		format (layout as string) to:file
		close file
	)
)

--gc light:false delayed:false
saveSceneInfo()
