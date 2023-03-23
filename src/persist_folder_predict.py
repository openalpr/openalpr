import time
import os
import subprocess
import logging
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from queue import Queue, Empty


class PlateProcessor(FileSystemEventHandler):
    def __init__(self, dir_path, flags, log_name, queue):
        self.dir_path = dir_path
        self.flags = flags
        self.log_name = log_name
        self.queue = queue

    def on_created(self, event):
        if event.is_directory:
            return None

        if not event.src_path.endswith('.jpg'):
            return None

        # Add the new image file to the queue
        self.queue.put(event.src_path)


def process_image(file_path, flags, log_name):
    if not os.path.exists(f'/logs/sent_plates/{log_name}.log'):
        with open(f'/logs/sent_plates/{log_name}.log', 'w') as f:
            pass

    filename = os.path.basename(file_path)
    cmd = f'echo {filename} >> /logs/processed_plates/{log_name}.log && alpr {flags} "{file_path}" >> /logs/processed_plates/{log_name}.log'
    subprocess.run(['sh', '-c', cmd])
    with open(f'/logs/sent_plates/{log_name}.log', 'a') as f:
        f.write(f'{filename}\n')


def main():
    dir_path = '/detect'
    flags = '-c brg -p gn -j'

    while not os.listdir(dir_path):
        time.sleep(0.5)

    folders = sorted(next(os.walk(dir_path))[1])
    log_name = folders[-1] if folders[-1] is not None else "default"
    file_path = dir_path + '/' + log_name + "/crops/placa carro"

    while not os.path.exists(file_path):
        time.sleep(0.5)

    # Create a queue to hold file paths of newly added images
    queue = Queue()

    # Watch for new images in /data using watchdog
    event_handler = PlateProcessor(file_path, flags, log_name, queue)
    observer = Observer()
    observer.schedule(event_handler, file_path, recursive=True)
    observer.start()

    try:
        while True:
            try:
                # Process the next image file in the queue
                file_path = queue.get(timeout=1)
                if file_path.endswith('.jpg'):
                    process_image(file_path, flags, log_name)
            except Empty:
                pass
    except KeyboardInterrupt:
        observer.stop()

    observer.join()


if __name__ == '__main__':
    time.sleep(5)
    main()