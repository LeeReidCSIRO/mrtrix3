/*
    Copyright 2012 Brain Research Institute, Melbourne, Australia

    Written by David Raffelt 17/02/12.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef __image_filter_gradient_h__
#define __image_filter_gradient_h__

#include "image/buffer_scratch.h"
#include "image/info.h"
#include "image/loop.h"
#include "image/threaded_copy.h"
#include "image/transform.h"
#include "image/adapter/gradient1D.h"
#include "image/filter/base.h"
#include "image/filter/base.h"

namespace MR
{
  namespace Image
  {
    namespace Filter
    {
      /** \addtogroup Filters
      @{ */

      /*! Compute the image gradients of a 3D or 4D image.
       *
       * Typical usage:
       * \code
       * Image::BufferPreload<float> src_data (argument[0]);
       * Image::BufferPreload<float>::voxel_type src (src_data);
       * Image::Filter::Gradient gradient_filter (src);
       *
       * Image::Header header (src_data);
       * header.info() = gradient_filter.info();
       * header.datatype() = src_data.datatype();
       *
       * Image::Buffer<float> dest_data (argument[1], src_data);
       * Image::Buffer<float>::voxel_type dest (dest_data);
       *
       * smooth_filter (src, dest);
       *
       * \endcode
       */
      class Gradient : public Base
      {
        public:
          template <class InfoType>
          Gradient (const InfoType& in) :
              Base (in),
              smoother (in),
              wrt_scanner_ (true)
          {
            if (in.ndim() == 4) {
              axes_.resize (5);
              axes_[3].dim = 3;
              axes_[4].dim = in.dim(3);
              axes_[0].stride = 2;
              axes_[1].stride = 3;
              axes_[2].stride = 4;
              axes_[3].stride = 1;
              axes_[4].stride = 5;
            } else if (in.ndim() == 3) {
              axes_.resize (4);
              axes_[3].dim = 3;
              axes_[0].stride = 2;
              axes_[1].stride = 3;
              axes_[2].stride = 4;
              axes_[3].stride = 1;
            } else {
              throw Exception("input image must be 3D or 4D");
            }
            datatype_ = DataType::Float32;
          }

          void compute_wrt_scanner (bool wrt_scanner) {
            wrt_scanner_ = wrt_scanner;
          }

          void set_stdev (const std::vector<float>& stdevs)
          {
            smoother.set_stdev (stdevs);
          }


          template <class InputVoxelType, class OutputVoxelType>
          void operator() (InputVoxelType& in, OutputVoxelType& out) {

              Image::BufferScratch<float> smoothed_data (smoother.info());
              Image::BufferScratch<float>::voxel_type smoothed_voxel (smoothed_data);
              smoother (in, smoothed_voxel);

              const size_t num_volumes = (in.ndim() == 3) ? 1 : in.dim(3);

              for (size_t vol = 0; vol < num_volumes; ++vol) {
                if (in.ndim() == 4) {
                  in[3] = vol;
                  out[4] = vol;
                }
                Adapter::Gradient1D<Image::BufferScratch<float>::voxel_type> gradient1D (smoothed_voxel);
                out[3] = 0;
                threaded_copy (gradient1D, out, 2, 0, 3);
                out[3] = 1;
                gradient1D.set_axis (1);
                threaded_copy (gradient1D, out, 2, 0, 3);
                out[3] = 2;
                gradient1D.set_axis (2);
                threaded_copy (gradient1D, out, 2, 0, 3);

                if (wrt_scanner_) {
                  Image::Transform transform (in);

                  Math::Vector<float> gradient (3);
                  Math::Vector<float> gradient_wrt_scanner (3);

                  Image::Loop loop (0, 3);
                  for (loop.start (out); loop.ok(); loop.next (out)) {
                    for (size_t dim = 0; dim < 3; dim++) {
                      out[3] = dim;
                      gradient[dim] = out.value() / in.vox(dim);
                    }
                    transform.image2scanner_dir (gradient, gradient_wrt_scanner);
                    for (size_t dim = 0; dim < 3; dim++) {
                      out[3] = dim;
                      out.value() = gradient_wrt_scanner[dim];
                    }
                  }
                }
              }
          }

        protected:
          Image::Filter::GaussianSmooth<> smoother;
          bool wrt_scanner_;
      };
      //! @}
    }
  }
}

/*
    Image::Filter::GaussianSmooth<> smooth_filter (input_voxel);
    Image::Filter::Gradient gradient_filter (input_voxel);

    Image::Header smooth_header (input_data);
    smooth_header.info() = smooth_filter.info();

    Image::BufferScratch<float> smoothed_data (smooth_header);
    Image::BufferScratch<float>::voxel_type smoothed_voxel (smoothed_data);

    Image::Header output_header (input_data);
    output_header.info() = gradient_filter.info();

    Image::Buffer<float> output_data (argument[2], output_header);
    Image::Buffer<float>::voxel_type output_voxel (output_data);

    smooth_filter (input_voxel, smoothed_voxel);
    gradient_filter (smoothed_voxel, output_voxel);
*/

#endif
