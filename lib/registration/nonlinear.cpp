/*
 * Copyright (c) 2008-2016 the MRtrix3 contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * MRtrix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * For more details, see www.mrtrix.org
 *
 */

#include "registration/nonlinear.h"

namespace MR
{
  namespace Registration
  {

    using namespace App;

    const OptionGroup nonlinear_options =
      OptionGroup ("Non-linear registration options")

      + Option ("nl_warp", "the non-linear output defined as four displacement fields in midway space. The 4th image dimension defines x,y,z component, "
                            "and the 5th dimension defines the field in this order (image1->midway, midway->image1, image2->midway, midway->image2)."
                            "Where image1->midway defines the field that maps image1 onto the midway space using the reverse convention (i.e. displacements map midway voxel positions to image1 space)."
                            "When linear registration is performed first, the estimated linear transform will be included in the comments of the image header, and therefore the entire linear and "
                            "non-linear transform can be applied using this output warp file with mrtransform")
        + Argument ("image").type_file_out ()

      + Option ("nl_init", "initialise the non-linear registration with the supplied warp image. The supplied warp must be in the same format as output using the -nl_warp option "
                            "(i.e. have 4 displacement fields with the linear transform in the image header)")
        + Argument ("image").type_image_in ()

      + Option ("nl_scale", "use a multi-resolution scheme by defining a scale factor for each level "
                             "using comma separated values (Default: 0.25,0.5,1.0)")
        + Argument ("factor").type_sequence_float ()

      + Option ("nl_niter", "the maximum number of iterations. This can be specified either as a single number "
                             "for all multi-resolution levels, or a single value for each level. (Default: 50)")
        + Argument ("num").type_sequence_int ()

      + Option ("nl_update_smooth", "regularise the gradient update field with Gaussian smoothing (standard deviation in voxel units, Default 2.0 x voxel_size)")
        + Argument ("stdev").type_float ()

      + Option ("nl_disp_smooth", "regularise the displacement field with Gaussian smoothing (standard deviation in voxel units, Default 1.0 x voxel_size)")
        + Argument ("stdev").type_float ()

      + Option ("nl_grad_step", "the gradient step size for non-linear registration (Default: 0.5)")
        + Argument ("num").type_float ();

  }
}

