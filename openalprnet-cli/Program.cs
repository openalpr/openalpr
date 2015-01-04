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
using System.IO;
using System.Linq;
using System.Reflection;
using openalprnet;

namespace openalprnet_cli
{
    internal class Program
    {
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

        private static void Main(string[] args)
        {
            Console.WriteLine(AlprNet.getVersion());
            var config = Path.Combine(AssemblyDirectory, "openalpr.conf");
            var alpr = new AlprNet("us", config);
            var loaded = alpr.isLoaded();
            var samplePath = Path.Combine(AssemblyDirectory, @"samples\us-1.jpg");
            var imgBytes = File.ReadAllBytes(samplePath);
            //alpr.TopN = 3;
            alpr.DefaultRegion = "us";
            alpr.DetectRegion = true;
            //var results = alpr.recognize(samplePath);
            var results = alpr.recognize(imgBytes);
            var json = alpr.toJson();
            Console.WriteLine(json);
            var plates = results.First().topNPlates;
        }
    }
}