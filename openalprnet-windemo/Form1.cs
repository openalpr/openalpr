/*
 * Copyright (c) 2015 Dr. Masroor Ehsan
 *
 * This file is part of OpenAlpr.Net.
 *
 * OpenAlpr.Net is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License
 * version 3 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using openalprnet;

namespace openalprnet_windemo
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        public static string AssemblyDirectory
        {
            get
            {
                var codeBase = Assembly.GetExecutingAssembly().CodeBase;
                var uri = new UriBuilder(codeBase);
                var path = Uri.UnescapeDataString(uri.Path);
                return Path.GetDirectoryName(path);
            }
        }

        public Rectangle boundingRectangle(List<Point> points)
        {
            // Add checks here, if necessary, to make sure that points is not null,
            // and that it contains at least one (or perhaps two?) elements

            var minX = points.Min(p => p.X);
            var minY = points.Min(p => p.Y);
            var maxX = points.Max(p => p.X);
            var maxY = points.Max(p => p.Y);

            return new Rectangle(new Point(minX, minY), new Size(maxX - minX, maxY - minY));
        }

        private static Image cropImage(Image img, Rectangle cropArea)
        {
            var bmpImage = new Bitmap(img);
            return bmpImage.Clone(cropArea, bmpImage.PixelFormat);
        }

        public static Bitmap combineImages(List<Image> images)
        {
            //read all images into memory
            Bitmap finalImage = null;

            try
            {
                int width = 0;
                int height = 0;

                foreach (var bmp in images)
                {
                    width += bmp.Width;
                    height = bmp.Height > height ? bmp.Height : height;
                }

                //create a bitmap to hold the combined image
                finalImage = new System.Drawing.Bitmap(width, height);

                //get a graphics object from the image so we can draw on it
                using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(finalImage))
                {
                    //set background color
                    g.Clear(System.Drawing.Color.Black);

                    //go through each image and draw it on the final image
                    int offset = 0;
                    foreach (System.Drawing.Bitmap image in images)
                    {
                        g.DrawImage(image,
                          new System.Drawing.Rectangle(offset, 0, image.Width, image.Height));
                        offset += image.Width;
                    }
                }

                return finalImage;
            }
            catch (Exception ex)
            {
                if (finalImage != null)
                    finalImage.Dispose();

                throw ex;
            }
            finally
            {
                //clean up memory
                foreach (System.Drawing.Bitmap image in images)
                {
                    image.Dispose();
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (openFileDialog.ShowDialog(this) == DialogResult.OK)
            {
                processImageFile(openFileDialog.FileName);
            }
        }

        private void processImageFile(string fileName)
        {
            resetControls();
            var region = rbUSA.Checked ? "us" : "eu";
            using (var alpr = new AlprNet(region, Path.Combine(AssemblyDirectory, "openalpr.conf")))
            {
                picOriginal.ImageLocation = fileName;
                picOriginal.Load();

                var results = alpr.recognize(fileName);

                List<Image> images = new List<Image>(results.Count());
                var i = 1;
                foreach (var result in results)
                {
                    var rect = boundingRectangle(result.plate_points);
                    var img = Image.FromFile(fileName);
                    var cropped = cropImage(img, rect);
                    images.Add(cropped);
                    
                    lbxPlates.Items.Add("\t\t-- Plate #" + i++ + " --");
                    foreach (var plate in result.topNPlates)
                    {
                        lbxPlates.Items.Add(string.Format(@"{0} {1}% {2}",
                                                          plate.characters.PadRight(12),
                                                          plate.overall_confidence.ToString("N1").PadLeft(8),
                                                          plate.matches_template.ToString().PadLeft(8)));
                    }
                }
                
                if (images.Any())
                {
                    picLicensePlate.Image = combineImages(images);
                }
            }
        }

        private void resetControls()
        {
            picOriginal.Image = null;
            picLicensePlate.Image = null;
            lbxPlates.Items.Clear();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            resetControls();
        }
    }
}