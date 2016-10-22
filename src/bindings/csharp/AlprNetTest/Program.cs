using System;


namespace AlprNetTest
{
    class Program
    {
        static void Main(string[] args)
        {
            
            // Create the object
            AlprNet.Alpr alpr = new AlprNet.Alpr("us");

            // Initialize the OpenALPR runtime data.  This must be called only once, and may take a few seconds to initialize
            bool success = alpr.Initialize();

            if (!success || !alpr.IsLoaded())
            {
                Console.WriteLine("OpenALPR was not properly initialized.  Exiting");
                alpr.Dispose();
                return;
            }

            alpr.setTopN(10);
            
            AlprNet.Models.AlprResponse response = alpr.Recognize(@"c:\projects\openalpr-commercial\image.jpg");

            Console.WriteLine("Analyzed an image that is {0}x{1} pixels", response.img_width, response.img_height);
            Console.WriteLine("Detected {0} plates", response.results.Count);

            foreach (AlprNet.Models.AlprResult result in response.results)
            {
                Console.WriteLine("  - {0} ({1}%)", result.plate, result.confidence);

                Console.WriteLine("  Top N candidates:");
                foreach (AlprNet.Models.Candidate candidate in result.candidates)
                {
                    Console.WriteLine("    - {0} ({1}%)", candidate.plate, candidate.confidence);
                }
            }

            // Free the OpenALPR instance's memory.
            alpr.Dispose();

            Console.WriteLine("\nPress any key to continue...");
            Console.ReadKey();
        }
    }
}
