/*
    Copyright 2009 Brain Research Institute, Melbourne, Australia

    Written by Robert E. Smith, 29/09/12.

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

#include "file/utils.h"
#include "file/path.h"
#include "file/gz.h"
#include "file/mgh_utils.h"
#include "image/utils.h"
#include "image/header.h"
#include "image/handler/gz.h"
#include "image/format/list.h"

namespace MR
{
  namespace Image
  {
    namespace Format
    {


      RefPtr<Handler::Base> MGZ::read (Header& H) const
      {
        if (!Path::has_suffix (H.name(), ".mgh.gz") && !Path::has_suffix (H.name(), ".mgz"))
          return RefPtr<Handler::Base>();

        mgh_header MGHH;
        mgh_other MGHO;

        File::GZ zf (H.name(), "rb");
        zf.read (reinterpret_cast<char*> (&MGHH), MGH_HEADER_SIZE);

        const size_t data_offset = File::MGH::read_header (H, MGHH, MGH_HEADER_SIZE);
        //const size_t other_offset = data_offset + Image::footprint (H);
        // TODO Have to read the relevat post-data header information into local memory
        //   so that it can be parsed by File::MGH::read_other()

        zf.close();

        RefPtr<Handler::Base> handler (new Handler::GZ (H, 0));
        handler->files.push_back (File::Entry (H.name(), data_offset));

        return handler;
      }





      bool MGZ::check (Header& H, size_t num_axes) const
      {
        if (!Path::has_suffix (H.name(), ".nii.gz") && !Path::has_suffix (H.name(), ".mgz")) return false;
        if (num_axes < 3) throw Exception ("cannot create MGZ image with less than 3 dimensions");
        if (num_axes > 4) throw Exception ("cannot create MGZ image with more than 4 dimensions");

        H.set_ndim (num_axes);

        return true;
      }





      RefPtr<Image::Handler::Base> MGZ::create (Header& H) const
      {
        if (H.ndim() > 4)
          throw Exception ("MGZ format cannot support more than 4 dimensions for image \"" + H.name() + "\"");

        RefPtr<Handler::GZ> handler (new Handler::GZ (H, MGH_DATA_OFFSET));

        File::MGH::write_header (*reinterpret_cast<mgh_header*> (handler->header()), H);

        // TODO Figure out how to write the post-data header information to the zipped file

        File::create (H.name());
        handler->files.push_back (File::Entry (H.name(), MGH_DATA_OFFSET));

        return handler;
      }

    }
  }
}

