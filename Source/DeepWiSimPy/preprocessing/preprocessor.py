"""
The incoming data will first save in preprocessor storage queue,
When data size reach given amount. dequeue and preprocess data for training
The data will sent to trainer to train the model.
"""
import csv
import queue
import time
import settings
import os

from thread.runnable import Runnable

NUM_OF_SOURCES = settings.UNREAL_CONFIG['num_of_beacons']
CSV_FILE = settings.APP_CONFIG['preprocessed_csv_file']


class Preprocessor(Runnable):
    def __init__(self, wait_time=0.01, on_preprocessed_fn=None):
        super(Preprocessor, self).__init__(wait_time=wait_time)
        self.on_preprocessed_fn = on_preprocessed_fn
        self.data_queue = queue.Queue()
        self.csv_fields = ['x', 'y', 'z', 'timestamp'] + [f'Beacon{i+1}' for i in range(NUM_OF_SOURCES)]
        self.prepare_csv()
        self.print("start.")

    def prepare_csv(self):
        if not os.path.exists(CSV_FILE):
            with open(CSV_FILE, 'w', newline='') as f:
                w = csv.DictWriter(f, fieldnames=self.csv_fields)
                w.writeheader()
                f.close()

    def do(self):
        if not self.data_queue.empty():
            data = self.data_queue.get()
            self.preprocess(data)

    def enqueue(self, data):
        self.data_queue.put(data)

    def preprocess(self, data):
        self.write_csv(data)

    def write_csv(self, data):
        with open(CSV_FILE, 'a', newline='') as f:
            w = csv.DictWriter(f, fieldnames=self.csv_fields)
            w.writerow(self.convert_to_csv_row(data))
            f.close()

    def convert_to_csv_row(self, data):
        csv_row = {'x': data['coordinates']['x'],
                   'y': data['coordinates']['y'],
                   'z': data['coordinates']['z'],
                   'timestamp': data['timestamp']}
        for i in range(1, len(data['measurement']) + 1):
            key = f'Beacon{i}'
            csv_row[key] = data['measurement'][key]
        return csv_row


if __name__ == "__main__":

    preprocessor = Preprocessor(wait_time=0.01)
    preprocessor.start()

    while True:
        time.sleep(60)
