#!/usr/bin/env python3

import beanstalkc

beanstalk = beanstalkc.Connection(host='localhost', port=11300)

beanstalk.watch('alpr')

job = beanstalk.reserve()

print(job.body)

job.delete()
