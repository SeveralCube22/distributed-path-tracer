#include "path_tracer/image/image.hpp"
#include "path_tracer/math/math.hpp"

using namespace math;

namespace image {
	image::image(uvec2 size, uint32_t channel_count, bool hdr, bool srgb) :
		size(size),
		channel_count(channel_count),
		hdr(hdr), srgb(srgb) {
		assert((size.x != 0 && size.y != 0)
			&& "Image size can't be zero.");
		assert(channel_count != 0
			&& "At least one channel is required.");
		assert(channel_count < 5
			&& "No more than 4 channels are supported.");
		assert(!hdr || !srgb
			&& "HDR image can't be sRGB.");

		data.resize(size.x * size.y * channel_count * (hdr ? 4 : 1), 0x0);
	}

	std::shared_ptr<image> image::load(const std::filesystem::path& path, bool srgb) {
		auto img = std::shared_ptr<image>(new image());

		img->hdr = stbi_is_hdr(path.string().c_str());
		img->srgb = srgb;

		uint8_t* data;
		if (img->hdr) {
			data = reinterpret_cast<uint8_t*>(stbi_loadf(path.string().c_str(),
			                                             reinterpret_cast<int32_t*>(&img->size.x),
			                                             reinterpret_cast<int32_t*>(&img->size.y),
			                                             reinterpret_cast<int32_t*>(&img->channel_count), 0));
		}
		else {
			const char* p = path.string().c_str();
			data = stbi_load(path.string().c_str(),
			                 reinterpret_cast<int32_t*>(&img->size.x),
			                 reinterpret_cast<int32_t*>(&img->size.y),
			                 reinterpret_cast<int32_t*>(&img->channel_count), 0);
		}

		if (data == nullptr)
			throw std::runtime_error("Failed to load image to memory: " + path.string());

		uint32_t byte_length = img->size.x * img->size.y
			* img->channel_count * (img->hdr ? 4 : 1);
		img->data.resize(byte_length);
		std::memcpy(img->data.data(), data, byte_length);

		stbi_image_free(data);
		return img;
	}

	std::shared_ptr<image> image::load_from_memory(const std::vector<uint8_t>& buffer_data, bool srgb) {
		auto img = std::shared_ptr<image>(new image());

		img->hdr = stbi_is_hdr_from_memory(buffer_data.data(), buffer_data.size());
		img->srgb = srgb;

		uint8_t* data;
		if (img->hdr) {
			data = reinterpret_cast<uint8_t*>(stbi_load_from_memory(buffer_data.data(), buffer_data.size(),
			                                             reinterpret_cast<int32_t*>(&img->size.x),
			                                             reinterpret_cast<int32_t*>(&img->size.y),
			                                             reinterpret_cast<int32_t*>(&img->channel_count), 0));
		}
		else {
			data = stbi_load_from_memory(buffer_data.data(), buffer_data.size(),
			                 reinterpret_cast<int32_t*>(&img->size.x),
			                 reinterpret_cast<int32_t*>(&img->size.y),
			                 reinterpret_cast<int32_t*>(&img->channel_count), 0);
		}

		if (data == nullptr)
			throw std::runtime_error("Failed to load image from memory");

		uint32_t byte_length = img->size.x * img->size.y
			* img->channel_count * (img->hdr ? 4 : 1);

		img->data.resize(byte_length);
		std::memcpy(img->data.data(), data, byte_length);

		stbi_image_free(data);
		return img;
	}

	void image::save(const std::filesystem::path& path) const {
		std::filesystem::create_directories(path.parent_path());

		std::string ext = path.extension().string();

		if (ext == ".png") {
			if (hdr)
				throw std::invalid_argument("Can't save HDR image as PNG.");

			stbi_write_png(path.string().c_str(), size.x, size.y,channel_count, data.data(), size.x * channel_count);
		}
		else if (ext == ".hdr") {
			if (hdr)
				throw std::invalid_argument("Can't save LDR image as HDR.");

			stbi_write_hdr(path.string().c_str(), size.x, size.y, channel_count,
			               reinterpret_cast<const float*>(data.data()));
		}
		else
			throw std::invalid_argument("Unknown file extension.");
	}

	std::vector<uint8_t> image::save_to_memory_png() const {
		int output_length;
		unsigned char* output = stbi_write_png_to_mem(data.data(), size.x * channel_count, size.x, size.y, channel_count, &output_length);

		if (output == nullptr)
			throw std::runtime_error("Failed to save image to memory.");

		std::vector<uint8_t> png_data(output, output + output_length);
        free(output);

        return png_data;
	}

	float image::read(const uvec2& pos, uint32_t channel) const {
		uint32_t index = pos.y * size.x + pos.x;
		index = index * channel_count + channel;

		float value;

		if (hdr) {
			value = *reinterpret_cast<const float*>(
				data.data() + (index * 4));
		}
		else
			value = data[index] / 255.0F;

		if (srgb && channel < 3)
			value = math::pow(value, 2.2F);

		return value;
	}

	void image::write(const uvec2& pos, uint32_t channel, float value) {
		if (srgb && channel < 3)
			value = math::pow(value, 1 / 2.2F);

		uint32_t index = pos.y * size.x + pos.x;
		index = index * channel_count + channel;

		if (hdr)
			std::memcpy(data.data() + (index * 4), &value, 4);
		else
			data[index] = static_cast<uint8_t>(value * 255 + 0.5F);
	}

	const math::uvec2& image::get_size() const {
		return size;
	}

	uint32_t image::get_channel_count() const {
		return channel_count;
	}

	bool image::is_hdr() const {
		return hdr;
	}

	bool image::is_srgb() const {
		return srgb;
	}
}
