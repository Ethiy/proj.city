#pragma once

#include <io/io.h>

#include <projection/raster_projection.h>

#include <ogrsf_frmts.h>

#include <map>
#include <vector>
#include <string>

namespace city
{
    namespace io
    {
        class RasterHandler: protected FileHandler
        {
        public:
            RasterHandler(boost::filesystem::path const& _filepath, std::map<std::string, bool> const& _modes);
            ~RasterHandler(void);

            projection::RasterPrint read(void);

            void write(projection::RasterPrint const& raster_image);
        };
    }
}
