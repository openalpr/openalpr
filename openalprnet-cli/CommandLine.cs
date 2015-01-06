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
using System.Linq;
using System.Text.RegularExpressions;

namespace openalprnet_cli
{
    internal static class CommandLine
    {
        private const string NameGroup = "name"; // Names of capture groups
        private const string ValueGroup = "value";
        /* The regex that extracts names and comma-separated values for switches 
        in the form (<switch>[="value 1",value2,...])+ */

        private static readonly Regex RexPattern =
            new Regex(@"(?<name>[^=]+)=?((?<quoted>\""?)(?<value>(?(quoted)[^\""]+|[^,]+))\""?,?)*",
                      RegexOptions.Compiled | RegexOptions.CultureInvariant |
                      RegexOptions.ExplicitCapture | RegexOptions.IgnoreCase);

        public static void Process(this string[] args, Action printUsage, params Switch[] switches)
        {
            /* Run through all matches in the argument list and if any of the switches 
            match, get the values and invoke the handler we were given. We do a Sum() 
            here for 2 reasons; a) To actually run the handlers
            and b) see if any were invoked at all (each returns 1 if invoked).
            If none were invoked, we simply invoke the printUsage handler. */
            if ((from arg in args
                 from Match match in RexPattern.Matches(arg)
                 from s in switches
                 where match.Success &&
                       ((string.Compare(match.Groups[NameGroup].Value, s.Name, true) == 0) ||
                        (string.Compare(match.Groups[NameGroup].Value, s.ShortForm, true) == 0))
                 select s.InvokeHandler(match.Groups[ValueGroup].Value.Split(','))).Sum() == 0)
                printUsage(); // We didn't find any switches
        }

        public class Switch // Class that encapsulates switch data.
        {
            public Switch(string name, Action<IEnumerable<string>> handler, string shortForm)
            {
                Name = name;
                Handler = handler;
                ShortForm = shortForm;
            }

            public Switch(string name, Action<IEnumerable<string>> handler)
            {
                Name = name;
                Handler = handler;
                ShortForm = null;
            }

            public string Name { get; private set; }
            public string ShortForm { get; private set; }
            public Action<IEnumerable<string>> Handler { get; private set; }

            public int InvokeHandler(string[] values)
            {
                Handler(values);
                return 1;
            }
        }
    }
}