#!/usr/bin/env python
# coding: utf-8

# Configuracion inicial
import pandas  # libreria para procesamiento de datos muy usada en la industria
import matplotlib.pyplot as plt  # librería para graficas de python
import json
import seaborn  # Extiende las funcionalidades de matplotlib, muy usado en data visualization
import numpy  # extiende las librerias de matemática de python

# Con esto hacemos los gráficos más grandes, adecuados para el proyector.
seaborn.set_context(context='talk', font_scale=1.2)

TOTAL_TIME = 200
NODE_AMMOUNT = 8


class DfWrapper:
    def __init__(self, file_path):
        self.file_path = file_path
        self.data = None

        self.set_data_from_json()

    def load_json_file(self, file_path):
        with open(file_path) as json_file:
            data = json.load(json_file)
        return data

    def get_inter_arrival_time(self, item):
        value = item['config'][4]["Network.node[{0,1,2,3,4,6,7}].app.interArrivalTime"]
        return float(value.split("(")[-1][:-1])

    def get_inter_arrival_times(self):
        result = [
            self.get_inter_arrival_time(row) for _, row in self.data.iterrows()]
        print(result)
        return result

    def sort_json(self, data):
        data = list(data)
        return sorted(data, key=self.get_inter_arrival_time)

    def set_data_from_json(self):
        json_data = self.load_json_file(self.file_path).values()
        sorted_data = self.sort_json(json_data)
        self.data = pandas.DataFrame.from_dict(sorted_data)

    def get_val_from_col(self, col, key):
        result = []
        for row in self.data[col]:
            for field in row:
                if field['name'] == key:
                    result.append(field['value'])
        return result


def get_delays(data):
    return data.get_val_from_col("scalars", "avgDelay")


def get_gen_intervals(data):
    interval_strings = data.get_val_from_col(
        "parameters", "generationInterval")
    return [float(interval.split("(")[-1][:-1]) for interval in interval_strings]


def draw_delay(data, index, rindex):
    gen_intervals, delays = get_gen_intervals(data), get_delays(data)
    print(gen_intervals)
    plt.plot(gen_intervals, delays, linewidth=0.5 + rindex*2,  zorder=index,
             label=f"{data.file_path.split('.json')[0]}")


def get_total_packets_by_key(data, key):
    packets = data.get_val_from_col("scalars", key)
    groups = [packets[i:i + NODE_AMMOUNT]
              for i in range(0, len(packets), NODE_AMMOUNT)]
    return [sum(group) for group in groups]


def draw_carga(data, index, rindex):
    paquetes_emisor = get_total_packets_by_key(data, "sentPackets")
    paquetes_receptor = get_total_packets_by_key(data, "receivedPackets")
    carga = [
        received/sent for sent, received in zip(paquetes_emisor, paquetes_receptor)
    ]
    print(carga)
    arriv_times = data.get_inter_arrival_times()
    plt.plot(arriv_times, carga, linewidth=0.5 + rindex*2, zorder=index,
             label=f"{data.file_path.split('.json')[0]}")


def draw_with_func(sources, draw_func):
    source_length = len(sources)
    for index, source in enumerate(sources):
        data = DfWrapper(source)
        draw_func(data, index, source_length - index)
    plt.legend(loc='best')
    plt.show()


def draw_delays(sources):
    plt.xlabel("generationInterval")
    plt.ylabel("Average Delay")
    draw_with_func(sources, draw_delay)


def draw_cargas(sources):
    plt.xlabel("interArrivalTime")
    plt.ylabel("Received/Sent")
    draw_with_func(sources, draw_carga)


def main():
    plt.style.use("ggplot")
    # caso1 = [
    #    "tc1p1.json",
    # ]
    caso2 = [
        "p1c2.json",
    ]
    # draw_delays(caso1)
    draw_cargas(caso2)


if __name__ == '__main__':
    main()
