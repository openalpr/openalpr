

namespace AlprNet.Models
{
    public class Candidate
    {
        public string plate { get; set; }
        public float confidence { get; set; }
        public bool matches_template { get; set; }
    }
}
