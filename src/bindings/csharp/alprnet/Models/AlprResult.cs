using System.Collections.Generic;


namespace AlprNet.Models
{
    public class AlprResult
    {
        public string plate { get; set; }
        public float confidence { get; set; }
        public bool matches_template { get; set; }
        public int plate_index { get; set; }
        public string region { get; set; }
        public float region_confidence { get; set; }
        public float processing_time_ms { get; set; }
        public int requested_topn { get; set; }
        public IList<Coordinate> coordinates { get; set; }
        public IList<Candidate> candidates { get; set; }
    }
}
