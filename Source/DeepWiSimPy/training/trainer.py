"""
Periodically read data from preprocessed file, use the latest data to train the model
"""

import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

import time
import pandas as pd
import numpy as np
import tensorflow as tf

import settings
from thread.runnable import Runnable
from preprocessing.dataset import DatasetGenerator
from nn.deepAR import create_model

CSV_FILE = settings.APP_CONFIG['preprocessed_csv_file']
CHECKPOINT_PATH = settings.APP_CONFIG['checkpoint_path']
HISTORY_PATH = settings.APP_CONFIG['training_history']
NUM_OF_BEACONS = settings.UNREAL_CONFIG['num_of_beacons']
EPOCHS = settings.APP_CONFIG['training_epochs']
DATA_SIZE = settings.APP_CONFIG['data_size']


class Trainer(Runnable):
    ready = False

    def __init__(self, input_width, label_width, shift, batch_size, partition=(0.8, 0.1, 0.1),
                 wait_time=0.01, on_training_completed_fn=None):
        super(Trainer, self).__init__(wait_time=wait_time)
        self.on_training_completed_fn = on_training_completed_fn
        self.input_width = input_width
        self.label_width = label_width
        self.shift = shift
        self.batch_size = batch_size
        self.partition = partition
        self.df = None
        self.dataset_generator = None
        self.print("start.")
        self.history = []

    def load(self):
        try:
            df = pd.read_csv(CSV_FILE)
            if len(df.index) > DATA_SIZE:
                df = df[-DATA_SIZE:]
            df = df.drop(columns=['timestamp'])
            self.df = (df-df.mean())/(df.std() + 1e-10)
        except FileNotFoundError:
            self.df = None
            self.dataset_generator = None
        # setup dataset generator
        if self.data_reach_min_size():
            columns = list(self.df.columns)
            self.dataset_generator = DatasetGenerator(data=self.df,
                                                      input_width=self.input_width,
                                                      label_width=self.label_width,
                                                      shift=self.shift,
                                                      columns=columns,
                                                      input_columns=columns[3:],
                                                      label_columns=columns[:3],
                                                      batch_size=self.batch_size,
                                                      partition=self.partition)
            self.print(f"Total data point: {len(self.df)}")

    def data_reach_min_size(self):
        total_row = len(self.df)
        seq_len = self.input_width + self.shift
        min_row = (seq_len + self.batch_size) / min(self.partition)
        return total_row > min_row

    def create_checkpoint_manager(self, model):
        ckpt = tf.train.Checkpoint(model=model)
        ckpt_manager = tf.train.CheckpointManager(ckpt, CHECKPOINT_PATH, max_to_keep=100)
        return ckpt, ckpt_manager

    def train(self):
        model = create_model(dim_x=NUM_OF_BEACONS, dim_z=3)
        # restore checkpoints if exist
        ckpt, ckpt_manager = self.create_checkpoint_manager(model)
        if ckpt_manager.latest_checkpoint:
            ckpt.restore(ckpt_manager.latest_checkpoint)
            self.print("Restoring checkpoint from {}".format(ckpt_manager.latest_checkpoint))
        # start training model
        train_result, test_result = {}, {}
        train_ds = self.dataset_generator.train_ds
        test_ds = self.dataset_generator.test_ds
        for i in range(EPOCHS):
            start = time.time()
            for data in train_ds:
                train_result = model.train_step(data)

            for data in test_ds:
                test_result = model.test_step(data)
            template = 'Epoch {}, Loss: {:.5f}, RMSE {:.5f}, Test Loss: {:.5f}, Test RMSE {:.5f}, ETC {:.5f} secs'
            self.print(template.format(i + 1,
                                       train_result['loss'],
                                       train_result['rmse'],
                                       test_result['loss'],
                                       test_result['rmse'],
                                       time.time() - start))
            self.log_history(training_loss=train_result['loss'],
                             training_rmse=train_result['rmse'],
                             test_loss=test_result['loss'],
                             test_rmse=test_result['rmse'])
            # save model parameters
            self.print('Saving checkpoint for epoch {} at {}'.format(1, ckpt_manager.save()))

    def do(self):
        self.load()
        if self.dataset_generator is not None:
            self.train()

    def log_history(self, training_loss, training_rmse, test_loss, test_rmse):
        self.history.append([training_loss, training_rmse, test_loss, test_rmse])
        np.savetxt(fname=HISTORY_PATH,
                   X=self.history,
                   delimiter=",",
                   header='training_loss,training_rmse,test_loss,test_rmse',
                   fmt='%f',
                   comments='')


if __name__ == "__main__":
    def on_training_completed():
        pass

    trainer = Trainer(on_training_completed_fn=on_training_completed)
    trainer.start()
    while True:
        time.sleep(60)
