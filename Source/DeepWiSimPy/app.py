import time
import json
import settings
from network.udp_client_unreal import UdpClientUnreal
from preprocessing.preprocessor import Preprocessor
from training.trainer import Trainer


def main():
    SERVER_ENDPOINT = settings.NETWORK_CONFIG['server_endpoint']
    CLIENT_ENDPOINT = settings.NETWORK_CONFIG['client_endpoint']

    def on_preprocessed():
        """ Callback function for finished preprocessing """
        pass

    prep = Preprocessor(on_preprocessed_fn=on_preprocessed)
    prep.start()

    def on_data_sent(byte_data, address):
        """ Callback function for sent data to server """
        pass

    def on_data_recv(byte_data, address):
        """ Callback function for received data from server """
        data = json.loads(byte_data.decode('utf-8'))
        prep.enqueue(data)

    client = UdpClientUnreal(server_endpoint=SERVER_ENDPOINT,
                             client_endpoint=CLIENT_ENDPOINT,
                             on_data_sent=on_data_sent,
                             on_data_recv=on_data_recv)
    client.start()

    def on_training_completed():
        """ Callback function for completed training """
        pass

    trainer = Trainer(input_width=64, label_width=64, shift=0, batch_size=64,
                      on_training_completed_fn=on_training_completed,
                      wait_time=0.01)
    trainer.start()

    while True:
        try:
            time.sleep(60)
        except KeyboardInterrupt:
            return

if __name__ == "__main__":
    main()
