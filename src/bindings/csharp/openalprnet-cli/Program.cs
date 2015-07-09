/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
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
using System.Diagnostics;
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

        private static bool StrToBool(string s)
        {
            return !string.IsNullOrEmpty(s) && s.Trim() == "1";
        }

        private static void Main(string[] args)
        {
            var region = "us";
            var detectRegion = false;
            var benchmark = false;
            var json = false;
            var filename = string.Empty;


            args.Process(
                () => Console.WriteLine("Usage: r=us/eu b=0/1 j=0/1 d=0/1 f=<filename>"),
                new CommandLine.Switch("r",
                                       val => { if (val.Any()) region = val.First().Trim().ToLower(); }),
                new CommandLine.Switch("b",
                                       val => { if (val.Any()) benchmark = StrToBool(val.First()); }),
                new CommandLine.Switch("j",
                                       val => { if (val.Any()) json = StrToBool(val.First()); }),
                new CommandLine.Switch("d",
                                       val => { if (val.Any()) detectRegion = StrToBool(val.First()); }),
                new CommandLine.Switch("f",
                                       val => { if (val.Any()) filename = val.First().Trim(); })
                );

            Console.WriteLine("OpenAlpr Version: {0}", AlprNet.GetVersion());
            var config = Path.Combine(AssemblyDirectory, "openalpr.conf");
            var runtime_data = Path.Combine(AssemblyDirectory, "runtime_data");
            var alpr = new AlprNet(region, config, runtime_data);
            if (!alpr.IsLoaded())
            {
                Console.WriteLine("OpenAlpr failed to load!");
                return;
            }

            //var samplePath = Path.Combine(AssemblyDirectory, @"samples\eu-1.jpg");
            //alpr.TopN = 3;
            alpr.DefaultRegion = region;
            alpr.DetectRegion = detectRegion;

            if (Directory.Exists(filename))
            {
                var files = Directory.GetFiles(filename, "*.jpg", SearchOption.TopDirectoryOnly);
                foreach (var fname in files)
                {
                    PerformAlpr(alpr, fname, benchmark, json);
                }
                return;
            }

            if (!File.Exists(filename))
            {
                Console.WriteLine("The file doesn't exist!");
                return;
            }
            var buffer = File.ReadAllBytes(filename);
            PerformAlpr(alpr, buffer, benchmark, json);
        }

        private static void PerformAlpr(AlprNet alpr, string filename, bool benchmark, bool writeJson)
        {
            Console.WriteLine("Processing '{0}'...\n------------------", Path.GetFileName(filename));
            var buffer = File.ReadAllBytes(filename);
            PerformAlpr(alpr, buffer, benchmark, writeJson);
        }

        private static void PerformAlpr(AlprNet alpr, byte[] buffer, bool benchmark, bool writeJson)
        {
            var sw = Stopwatch.StartNew();
            var results = alpr.Recognize(buffer);
            sw.Stop();
            if (benchmark)
            {
                Console.WriteLine("Total Time to process image(s): {0} msec(s)", sw.ElapsedMilliseconds);
            }

            if (writeJson)
            {
                //Console.WriteLine(alpr.toJson());
            }
            else
            {
                var i = 0;
                foreach (var result in results.Plates)
                {
                    Console.WriteLine("Plate {0}: {1} result(s)", i++, result.TopNPlates.Count);
                    Console.WriteLine("  Processing Time: {0} msec(s)", result.ProcessingTimeMs);
                    foreach (var plate in result.TopNPlates)
                    {
                        Console.WriteLine("  - {0}\t Confidence: {1}\tMatches Template: {2}", plate.Characters,
                                          plate.OverallConfidence, plate.MatchesTemplate);
                    }
                }
            }
        }
    }
}