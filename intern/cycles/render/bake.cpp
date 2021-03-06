/*
 * Copyright 2011-2014 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

#include "bake.h"

CCL_NAMESPACE_BEGIN

void BakeManager::device_free(Device *device, DeviceScene *dscene) {}

void BakeManager::device_update(Device *device, DeviceScene *dscene, Scene *scene, Progress& progress)
{
	if(!need_update)
		return;

	if(progress.get_cancel()) return;

	need_update = false;
}

BakeManager::~BakeManager()
{
	if(bake_data)
		delete bake_data;
}

BakeManager::BakeManager()
{
	bake_data = NULL;
	need_update = true;
}

BakeData *BakeManager::init(const int object, const int tri_offset, const int num_pixels)
{
	bake_data = new BakeData(object, tri_offset, num_pixels);
	return bake_data;
}

bool BakeManager::bake(Device *device, DeviceScene *dscene, Scene *scene, ShaderEvalType shader_type, BakeData *bake_data, float result[])
{
	size_t limit = bake_data->size();

	/* setup input for device task */
	device_vector<uint4> d_input;
	uint4 *d_input_data = d_input.resize(limit);
	size_t d_input_size = 0;

	for(size_t i = 0; i < limit; i++) {
		d_input_data[d_input_size++] = bake_data->data(i);
	}

	if(d_input_size == 0)
		return false;

	/* run device task */
	device_vector<float4> d_output;
	d_output.resize(d_input_size);

	/* needs to be up to data for attribute access */
	device->const_copy_to("__data", &dscene->data, sizeof(dscene->data));

	device->mem_alloc(d_input, MEM_READ_ONLY);
	device->mem_copy_to(d_input);
	device->mem_alloc(d_output, MEM_WRITE_ONLY);

	DeviceTask task(DeviceTask::SHADER);
	task.shader_input = d_input.device_pointer;
	task.shader_output = d_output.device_pointer;
	task.shader_eval_type = shader_type;
	task.shader_x = 0;
	task.shader_w = d_output.size();

	device->task_add(task);
	device->task_wait();

	device->mem_copy_from(d_output, 0, 1, d_output.size(), sizeof(float4));
	device->mem_free(d_input);
	device->mem_free(d_output);

//	if(progress.get_cancel())
//		return false;

	/* read result */
	int k = 0;

	float4 *offset = (float4*)d_output.data_pointer;

	size_t depth = 4;
	for(size_t i = 0; i < limit; i++) {
		size_t index = i * depth;
		float4 out = offset[k++];

		for(size_t j=0; j < 4; j++)
			result[index + j] = out[j];
	}

	return true;
}

CCL_NAMESPACE_END

