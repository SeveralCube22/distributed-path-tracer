#pragma once

#include "pch.hpp"
#include "processors/application.hpp"
#include "models/work_info.hpp"
#include "models/cloud_ray.hpp"
#include "cloud/s3.hpp"
#include "scene/scene.hpp"
#include <concurrentqueue/concurrentqueue.h>

namespace processors {
    class worker : public application {
    public:
        worker(const models::worker_info& worker_info);
        void run() override;
        ~worker() override;
    public:
		math::fvec3 environment_factor = math::fvec3::one;
		bool transparent_background = false;
        
    private:
        void download_gltf_file();

        void map_ray_stage_to_queue(const models::cloud_ray& ray);

        void process_intersections();
        void process_intersection_results();

        void process_direct_lighting(); // handle opacity, and direct lighting
        void process_direct_lighting_results(); // process result from direct lighting and handle indirect lighting

        void process_indirect_lighting_results(); // process result from indirect lighting
    
    private:
        models::worker_info m_worker_info;
        std::filesystem::path m_gltf_file_path;
        cloud::distributed_scene m_scene;

        std::atomic<bool> m_should_terminate;

        moodycamel::ConcurrentQueue<models::cloud_ray> m_intersection_queue;
        moodycamel::ConcurrentQueue<models::cloud_ray> m_intersection_result_queue;

        moodycamel::ConcurrentQueue<models::cloud_ray> m_direct_lighting_queue;
        moodycamel::ConcurrentQueue<models::cloud_ray> m_direct_lighting_result_queue;

        moodycamel::ConcurrentQueue<models::cloud_ray> m_indirect_lighting_queue;

        std::map<std::string, std::vector<models::cloud_ray>> m_intersection_results;
    };
}