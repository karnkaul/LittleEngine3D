#pragma once
#include <memory>
#include <vector>
#include "le3d/core/jobs.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/model.hpp"

namespace le
{
// Loads OBJ and MTL specified in a JSON into Model::Data via job workers;
// then loads each Model (via `resources`) on the main thread on every call to `loadNext()`,
// which returns `true` when all models have been loaded.
// JSON format:
//	{
//		"id": "<model name>"
//		"obj": "<.obj id>"
//		"mtl": "<.mtl id>"
//		"scale": "<model scale>"
//	}
// Location/ID: <idPrefix>/<id>.json
class AsyncModelsLoader
{
public:
	struct Data
	{
		using GetStream = std::function<std::stringstream(stdfs::path const&)>;
		using GetBytes = std::function<std::vector<u8>(stdfs::path const&)>;

		stdfs::path idPrefix;
		// id = <idPrefix>/<jsonRoot>.json
		std::vector<stdfs::path> jsonRoots;
		//	Passed <jsonRoot>/<jsonRoot>.json, <jsonRoot>/<obj>.obj, <jsonRoot>/<mtl>.mtl, should return stringstream of corresponding data
		GetStream getData;
		//	Passed texture ids in .mtl, should return bytestream (vector<u8>) of corresponding data
		GetBytes getBytes;
	};

protected:
	struct Request
	{
		Model::Data data;
		JobHandle hJob;
	};

protected:
	Data m_data;
	std::vector<std::shared_ptr<Request>> m_requests;
	bool m_bDone = false;

public:
	AsyncModelsLoader(Data data);
	virtual ~AsyncModelsLoader();

	bool loadNext(u16 count = 1);
	void waitAll();

	bool done() const;
};
} // namespace le
