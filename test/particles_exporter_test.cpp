#include "particles_exporter.hpp"

TEST(ParticlesExporterTest, isLittleEndian) {
    // This test is expected to pass on little-endian machines.
    ASSERT_TRUE(ParticlesExporter::isLittleEndian()); 
}