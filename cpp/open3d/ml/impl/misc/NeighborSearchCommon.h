// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2020 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#pragma once

#include <nanoflann.hpp>

#include "open3d/utility/Eigen.h"

namespace open3d {
namespace ml {
namespace detail {

/// Supported metrics
enum Metric { L1, L2, Linf };

#ifdef __CUDACC__
#define HOST_DEVICE __host__ __device__
#else
#define HOST_DEVICE
#endif

/// Spatial hashing function for integer coordinates.
HOST_DEVICE inline size_t SpatialHash(int x, int y, int z) {
    return x * 73856096 ^ y * 193649663 ^ z * 83492791;
}

inline size_t SpatialHash(const Eigen::Vector3i& xyz) {
    return SpatialHash(xyz.x(), xyz.y(), xyz.z());
}

/// Computes an integer voxel index for a 3D position.
///
/// \param pos               A 3D position.
/// \param inv_voxel_size    The reciprocal of the voxel size
///
template <class TDerived>
HOST_DEVICE inline Eigen::Vector3i ComputeVoxelIndex(
        const Eigen::ArrayBase<TDerived>& pos,
        const typename TDerived::Scalar& inv_voxel_size) {
    typedef typename TDerived::Scalar Scalar_t;
    Eigen::Array<Scalar_t, 3, 1> ref_coord = pos * inv_voxel_size;

    Eigen::Vector3i voxel_index;
    voxel_index = ref_coord.floor().template cast<int>();
    return voxel_index;
}
#undef HOST_DEVICE

/// Adaptor for nanoflann
template <class T>
class Adaptor {
public:
    Adaptor(size_t num_points, const T* const data)
        : num_points(num_points), data(data) {}

    inline size_t kdtree_get_point_count() const { return num_points; }

    inline T kdtree_get_pt(const size_t idx, int dim) const {
        return data[3 * idx + dim];
    }

    template <class BBOX>
    bool kdtree_get_bbox(BBOX&) const {
        return false;
    }

private:
    size_t num_points;
    const T* const data;
};

template <int METRIC, class T>
struct SelectNanoflannAdaptor {};

template <class T>
struct SelectNanoflannAdaptor<L2, T> {
    typedef nanoflann::L2_Adaptor<T, Adaptor<T>> Adaptor_t;
};

template <class T>
struct SelectNanoflannAdaptor<L1, T> {
    typedef nanoflann::L1_Adaptor<T, Adaptor<T>> Adaptor_t;
};

}  // namespace detail
}  // namespace ml
}  // namespace open3d
