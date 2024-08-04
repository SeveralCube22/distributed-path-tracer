#include "pch.hpp"

#include "core/pbr.hpp"
#include "core/renderer.hpp"
#include "image/image_texture.hpp"
#include "spdlog/spdlog.h"

#include <aws/lambda-runtime/runtime.h>
#include <aws/core/Aws.h>

using namespace math;

aws::lambda_runtime::invocation_response my_handler(aws::lambda_runtime::invocation_request const& request) {
	Aws::SDKOptions options;
   	Aws::InitAPI(options);
   	{
      // make your SDK calls here.
   	}
   	Aws::ShutdownAPI(options);

	core::renderer renderer;
	
	renderer.sample_count = 25;
	renderer.bounce_count = 4;
	renderer.resolution = uvec2(1920, 1080);

	renderer.environment_factor = fvec3::zero;
	renderer.transparent_background = true;

	renderer.load_gltf("scenes/cornell-box/cornell.gltf");

	renderer.render("renders/test.png");
	spdlog::info("Render Complete");
	return aws::lambda_runtime::invocation_response::success("Hello, World!", "application/json");
}


int main() {
	aws::lambda_runtime::run_handler(my_handler);
	return 0;
}
