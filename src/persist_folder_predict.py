import time
import os
import subprocess
import logging
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from queue import Queue

class PlateProcessor(FileSystemEventHandler):
    def __init__(self, dir_path, flags, log_name):
        self.dir_path = dir_path
        self.flags = flags
        self.log_name = log_name
        self.queue = Queue()

    def on_created(self, event):
        if event.is_directory:
            return None

        if not event.src_path.endswith('.jpg'):
            return None

        # Add new image file to the queue
        self.queue.put(event.src_path)

    def process_queue(self):
        while not self.queue.empty():
            # Process the next image file in the queue
            src_path = self.queue.get()
            filename = os.path.basename(src_path)
            cmd = f'echo {filename} >> /logs/processed_plates/{self.log_name}.log && alpr {self.flags} "{src_path}" >> /logs/processed_plates/{self.log_name}.log'
            subprocess.run(['sh', '-c', cmd])
            with open(f'/logs/sent_plates/{self.log_name}.log', 'a') as f:
                f.write(f'{os.path.basename(src_path)}\n')

    def on_modified(self, event):
        if event.is_directory:
            self.process_queue()

    def on_moved(self, event):
        if event.is_directory:
            self.process_queue()

def main():
    dir_path = '/detect'
    flags = '-c brg -p gn -j'

    while not os.listdir(dir_path):
        time.sleep(0.5)

    folders = sorted(next(os.walk(dir_path))[1])
    log_name = folders[-1] if folders[-1] is not None else "default"
    file_path = dir_path + '/' + log_name + "/crops/placa carro"

    if not os.path.exists(f'/logs/sent_plates/{log_name}.log'):
        with open(f'/logs/sent_plates/{log_name}.log', 'w') as f:
            pass

    while not os.path.exists(file_path):
        time.sleep(0.5)

    # Process existing files in the directory
    for filename in sorted(os.listdir(file_path)):
        if filename.endswith('.jpg'):
            cmd = f'echo {filename} >> /logs/processed_plates/{log_name}.log && alpr {flags} "{file_path}/{filename}" >> /logs/processed_plates/{log_name}.log'
            subprocess.run(['sh', '-c', cmd])
            with open(f'/logs/sent_plates/{log_name}.log', 'a') as f:
                f.write(f'{filename}\n')

    # Watch for new images in /data using watchdog
    event_handler = PlateProcessor(file_path, flags, log_name)
    observer = Observer()
    observer.schedule(event_handler, file_path, recursive=True)
    observer.start()

    try:
        while True:
            event_handler.process_queue()  # Process the queue periodically
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()

    observer.join()

if __name__ == '__main__':
    time.sleep(5)
    main()