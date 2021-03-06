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

#ifndef __BAKE_H__
#define __BAKE_H__

#include "util_vector.h"
#include "device.h"
#include "scene.h"
#include "session.h"

CCL_NAMESPACE_BEGIN

class BakeData {
public:
	BakeData(const int object, const int tri_offset, const int num_pixels):
	m_object(object),
	m_tri_offset(tri_offset),
	m_num_pixels(num_pixels)
	{
		m_primitive.resize(num_pixels);
		m_u.resize(num_pixels);
		m_v.resize(num_pixels);
	}

	~BakeData()
	{
		m_primitive.clear();
		m_u.clear();
		m_v.clear();
	}

	void set(int i, int prim, float uv[2])
	{
		m_primitive[i] = (prim == -1 ? -1 : m_tri_offset + prim);
		m_u[i] = uv[0];
		m_v[i] = uv[1];
	}

	int object()
	{
		return m_object;
	}

	int size() {
		return m_num_pixels;
	}

	uint4 data(int i) {
		return make_uint4(
			m_object,
			m_primitive[i],
			__float_as_int(m_u[i]),
			__float_as_int(m_v[i])
			);
	}


private:
	int m_object;
	int m_tri_offset;
	int m_num_pixels;
	vector<int>m_primitive;
	vector<float>m_u;
	vector<float>m_v;
};

class BakeManager {
public:

	bool need_update;

	BakeManager();
	~BakeManager();

	BakeData *init(const int object, const int tri_offset, const int num_pixels);

	bool bake(Device *device, DeviceScene *dscene, Scene *scene, ShaderEvalType shader_type, BakeData *bake_data, float result[]);

	void device_update(Device *device, DeviceScene *dscene, Scene *scene, Progress& progress);
	void device_free(Device *device, DeviceScene *dscene);
	bool modified(const CurveSystemManager& CurveSystemManager);
	void tag_update(Scene *scene);
	void tag_update_mesh();

private:
	BakeData *bake_data;
};

CCL_NAMESPACE_END

#endif /* __BAKE_H__ */

