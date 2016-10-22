using System.Collections.Generic;

namespace AlprNet.Models
{
    public class AlprResponse
    {
        public int version { get; set; }
        public string data_type { get; set; }
        public long epoch_time { get; set; }
        public int img_width { get; set; }
        public int img_height { get; set; }
        public float processing_time_ms { get; set; }
        public RegionOfInterest region_of_interest { get; set; }
        public IList<AlprResult> results { get; set; }
    }
}
