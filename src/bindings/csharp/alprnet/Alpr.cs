using System;
using System.Runtime.InteropServices;
using Newtonsoft.Json;

using AlprNet.Models;

namespace AlprNet
{
    public class Alpr : IDisposable
    {
        private const int REALLY_BIG_PIXEL_WIDTH = 999999999;
        private bool is_initialized = false;
        private string country;
        private string config_file;
        private string runtime_dir;
        private IntPtr native_instance;

        /// <summary>
        /// OpenALPR library constructor. The object must be initialized separately with a call to Initialize()
        /// </summary>
        /// <param name="country">Country used for recognition.  It may be a single country (e.g., "us") or a comma separated list</param>
        /// <param name="config_file">The path to the openalpr.conf file.  Leave it blank to use the file in the current directory.</param>
        /// <param name="runtime_dir">The path to the runtime_data directory.  Leave it blank to use the runtime_data in the current directory</param>
        public Alpr(string country,  string config_file = "", string runtime_dir = "")
        {
            this.country = country;
            this.config_file = config_file;
            this.runtime_dir = runtime_dir;
        }

        ~Alpr()
        {
            Dispose();
        }

        /// <summary>
        /// Release the memory associated with OpenALPR when you are done using it
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
        }

        protected virtual void Dispose(bool disposing)
        {

            // Deinitialize the library
            if (this.is_initialized)
            {
                try
                {
                    // Destroy the native obj
                    openalpr_cleanup(native_instance);
                }
                catch (System.DllNotFoundException)
                {
                    // Ignore.  The library couldn't possibly be loaded anyway
                }
            }
            this.is_initialized = false;
        }
        /// <summary>
        /// Load the OpenALPR recognition data into memory.
        /// Initialization may take several seconds to load all of the various LP recognition data into memory
        /// </summary>
        /// <returns>True if successful, false otherwise</returns>
        public bool Initialize()
        {
            // Initialize the library

            if (is_initialized)
            {
                // Idempotent behavior
                return true;
            }

            this.is_initialized = true;

            try
            {
                native_instance = openalpr_init(country, config_file, runtime_dir);
                openalpr_set_detect_region(native_instance, 1);
            }
            catch (System.DllNotFoundException)
            {
                Console.WriteLine("Could not find/load native library (libopenalprc.dll)");
                return false;
            }

            return IsLoaded();


        }
        
        /// <summary>
        /// Verifies that the OpenALPR library has been initialized
        /// </summary>
        /// <returns>true if the library is ready to use, false if it has not been initialized to read plates</returns>
        public bool IsLoaded()
        {
            if (!this.is_initialized)
                return false;

            try
            {
                return openalpr_is_loaded(this.native_instance) != 0;
            }
            catch (System.DllNotFoundException)
            {
                return false;
            }
        }

        // Checks if the instance can receive commands (e.g., is initialized)
        // If not, it throws an exception
        private void check_initialization()
        {
            if (!IsLoaded())
                throw new InvalidOperationException("OpenALPR Library cannot execute this request since it has not been initialized.");
        }

        /// <summary>
        /// Recognizes an encoded image (e.g., JPG, PNG, etc) from a file on disk
        /// </summary>
        /// <param name="filepath">The path to the image file</param>
        /// <returns>An AlprResponse object that contains the results of the recognition</returns>
        /// <exception cref="InvalidOperationException">Thrown when the library has not been initialized</exception>
        public AlprResponse Recognize(string filepath)
        {
            return Recognize(System.IO.File.ReadAllBytes(filepath));
        }

        /// <summary>
        /// Recognizes an encoded image (e.g., JPG, PNG, etc) provided as an array of bytes
        /// </summary>
        /// <param name="image_bytes">The raw bytes that compose the image</param>
        /// <returns>An AlprResponse object that contains the results of the recognition</returns>
        /// <exception cref="InvalidOperationException">Thrown when the library has not been initialized</exception>
        public AlprResponse Recognize(byte[] image_bytes)
        {
            check_initialization();

            NativeROI roi;
            roi.x = 0;
            roi.y = 0;
            roi.width = REALLY_BIG_PIXEL_WIDTH;
            roi.height = REALLY_BIG_PIXEL_WIDTH;

            IntPtr unmanagedArray = Marshal.AllocHGlobal(image_bytes.Length);
            Marshal.Copy(image_bytes, 0, unmanagedArray, image_bytes.Length);

            IntPtr resp_ptr = openalpr_recognize_encodedimage(this.native_instance, unmanagedArray, image_bytes.Length, roi);

            Marshal.FreeHGlobal(unmanagedArray);
            
            // Commented out test JSON string
            //string json = @"{""version"":2,""data_type"":""alpr_results"",""epoch_time"":1476716853320,""img_width"":600,""img_height"":600,""processing_time_ms"":116.557533,""regions_of_interest"":[{""x"":0,""y"":0,""width"":600,""height"":600}],""results"":[{""plate"":""627WWI"",""confidence"":94.338623,""matches_template"":1,""plate_index"":0,""region"":""wa"",""region_confidence"":82,""processing_time_ms"":50.445648,""requested_topn"":10,""coordinates"":[{""x"":242,""y"":360},{""x"":358,""y"":362},{""x"":359,""y"":412},{""x"":241,""y"":408}],""candidates"":[{""plate"":""627WWI"",""confidence"":94.338623,""matches_template"":1},{""plate"":""627WKI"",""confidence"":80.588486,""matches_template"":1},{""plate"":""627WI"",""confidence"":79.943542,""matches_template"":0},{""plate"":""627WVI"",""confidence"":79.348930,""matches_template"":1},{""plate"":""627WRI"",""confidence"":79.196785,""matches_template"":1},{""plate"":""627WNI"",""confidence"":79.165802,""matches_template"":1}]}]}";

            string json = Marshal.PtrToStringAnsi(resp_ptr);
            AlprResponse response = JsonConvert.DeserializeObject<AlprResponse>(json);
            openalpr_free_response_string(resp_ptr);

            return response;
        }

        /// <summary>
        /// Recognizes an image from a .NET Bitmap object
        /// </summary>
        /// <param name="bitmap">The .NET Bitmap object to recognize</param>
        /// <returns>An AlprResponse object that contains the results of the recognition</returns>
        /// <exception cref="InvalidOperationException">Thrown when the library has not been initialized</exception>
        public AlprResponse Recognize(System.Drawing.Image image)
        {
            check_initialization();

            System.Drawing.Bitmap clone = new System.Drawing.Bitmap(image.Width, image.Height, System.Drawing.Imaging.PixelFormat.Format32bppRgb);
            using (System.Drawing.Graphics gr = System.Drawing.Graphics.FromImage(clone))
            {
                gr.DrawImage(image, new System.Drawing.Rectangle(0, 0, clone.Width, clone.Height));
            }
            
            System.Drawing.Imaging.BitmapData locked_bmp = clone.LockBits(new System.Drawing.Rectangle(0, 0, clone.Width, clone.Height),
                 System.Drawing.Imaging.ImageLockMode.ReadWrite, clone.PixelFormat);
            byte[] raw_bytes = new byte[locked_bmp.Stride * locked_bmp.Height];
            System.Runtime.InteropServices.Marshal.Copy(locked_bmp.Scan0, raw_bytes, 0, raw_bytes.Length);
            clone.UnlockBits(locked_bmp);

            int bytes_per_pixel = System.Drawing.Image.GetPixelFormatSize(clone.PixelFormat) / 8;
            
            NativeROI roi;
            roi.x = 0;
            roi.y = 0;
            roi.width = image.Width;
            roi.height = image.Height;

            IntPtr unmanagedArray = Marshal.AllocHGlobal(raw_bytes.Length);
            Marshal.Copy(raw_bytes, 0, unmanagedArray, raw_bytes.Length);

            IntPtr resp_ptr = openalpr_recognize_rawimage(this.native_instance, unmanagedArray, bytes_per_pixel, image.Width, image.Height, roi);

            Marshal.FreeHGlobal(unmanagedArray);

            string json = Marshal.PtrToStringAnsi(resp_ptr);

            AlprResponse response = JsonConvert.DeserializeObject<AlprResponse>(json);

            openalpr_free_response_string(resp_ptr);

            return response;
        }

        /// <summary>
        /// Change the country used for analyzing plate data.  This will load the new training data automatically
        /// </summary>
        /// <param name="country">Country used for Recognition (e.g., "us", "eu", "au", etc).  Multiple values may be provided as comma-separated</param>
        /// <exception cref="InvalidOperationException">Thrown when the library has not been initialized</exception>
        public void SetCountry(string country)
        {
            check_initialization();

            openalpr_set_country(this.native_instance, country);
        }

        /// <summary>
        /// Applies a new prewarp configuration to this instance
        /// </summary>
        /// <param name="prewarp">A prewarp configuration string generated from the OpenALPR calibration utility</param>
        /// <exception cref="InvalidOperationException">Thrown when the library has not been initialized</exception>
        public void SetPrewarp(string prewarp)
        {
            openalpr_set_prewarp(this.native_instance, prewarp);
        }

        /// <summary>
        /// Set the detection mask used by OpenALPR.  Areas within the mask will be scanned for plates, areas outside the mask will be ignored
        /// </summary>
        /// <param name="mask">
        /// Provide a black and white image that contains white areas that should be scanned, and black areas that should be ignored. 
        /// The width and height should match the image dimensions used for recognition.
        /// </param>
        /// <exception cref="InvalidOperationException">Thrown when the library has not been initialized</exception>
        public void SetDetectionMask(System.Drawing.Image mask)
        {
            check_initialization();

            System.Drawing.Bitmap clone = new System.Drawing.Bitmap(mask.Width, mask.Height, System.Drawing.Imaging.PixelFormat.Format32bppRgb);
            using (System.Drawing.Graphics gr = System.Drawing.Graphics.FromImage(clone))
            {
                gr.DrawImage(mask, new System.Drawing.Rectangle(0, 0, clone.Width, clone.Height));
            }

            System.Drawing.Imaging.BitmapData locked_bmp = clone.LockBits(new System.Drawing.Rectangle(0, 0, clone.Width, clone.Height),
                 System.Drawing.Imaging.ImageLockMode.ReadWrite, clone.PixelFormat);
            byte[] raw_bytes = new byte[locked_bmp.Stride * locked_bmp.Height];
            System.Runtime.InteropServices.Marshal.Copy(locked_bmp.Scan0, raw_bytes, 0, raw_bytes.Length);
            clone.UnlockBits(locked_bmp);

            int bytes_per_pixel = System.Drawing.Image.GetPixelFormatSize(clone.PixelFormat) / 8;

            
            IntPtr unmanagedArray = Marshal.AllocHGlobal(raw_bytes.Length);
            Marshal.Copy(raw_bytes, 0, unmanagedArray, raw_bytes.Length);

            openalpr_set_mask(this.native_instance, unmanagedArray, bytes_per_pixel, mask.Width, mask.Height);

            Marshal.FreeHGlobal(unmanagedArray);
            
        }

        /// <summary>
        /// Sets the maximum number of results for OpenALPR to return with each recognition task
        /// </summary>
        /// <param name="top_n">An integer describing the maximum number of results to return</param>
        /// <exception cref="InvalidOperationException">Thrown when the library has not been initialized</exception>
        public void setTopN(int top_n)
        {
            check_initialization();

            openalpr_set_topn(this.native_instance, top_n);
        }

        /// <summary>
        /// Sets the region used by OpenALPR for pattern matching.  For example, "md" in the US would match plates matching that pattern.
        /// </summary>
        /// <param name="region">A two-letter code for the region/country to use as the default pattern</param>
        /// <exception cref="InvalidOperationException">Thrown when the library has not been initialized</exception>
        public void setDefaultRegion(string region)
        {
            check_initialization();

            openalpr_set_default_region(this.native_instance, region);
        }

        // Enumerate the native methods.  Handle the plumbing internally

        [StructLayout(LayoutKind.Sequential)]
        private struct NativeROI
        {
            public int x;
            public int y;
            public int width;
            public int height;
        }

        [DllImport("libopenalprc.dll")]
        private static extern IntPtr openalpr_init([MarshalAs(UnmanagedType.LPStr)] string country, [MarshalAs(UnmanagedType.LPStr)] string configFile, [MarshalAs(UnmanagedType.LPStr)] string runtimeDir);

        [DllImport("libopenalprc.dll")]
        private static extern int openalpr_is_loaded(IntPtr instance);

        [DllImport("libopenalprc.dll")]
        private static extern void openalpr_cleanup(IntPtr instance);

        [DllImport("libopenalprc.dll")]
        private static extern void openalpr_set_country(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string country);

        [DllImport("libopenalprc.dll")]
        private static extern void openalpr_set_prewarp(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string prewarp_config);

        [DllImport("libopenalprc.dll")]
        private static extern void openalpr_set_mask(IntPtr instance, IntPtr pixelData, int bytesPerPixel, int imgWidth, int imgHeight);

        [DllImport("libopenalprc.dll")]
        private static extern void openalpr_set_detect_region(IntPtr instance, int detectRegion);

        [DllImport("libopenalprc.dll")]
        private static extern void openalpr_set_topn(IntPtr instance, int topN);

        [DllImport("libopenalprc.dll")]
        private static extern void openalpr_set_default_region(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string region);

        [DllImport("libopenalprc.dll")]
        private static extern IntPtr openalpr_recognize_rawimage(IntPtr instance, IntPtr pixelData, int bytesPerPixel, int imgWidth, int imgHeight, NativeROI roi);

        [DllImport("libopenalprc.dll")]
        private static extern IntPtr openalpr_recognize_encodedimage(IntPtr instance, IntPtr bytes, long length, NativeROI roi);

        [DllImport("libopenalprc.dll")]
        private static extern void openalpr_free_response_string(IntPtr response);


    }
}
