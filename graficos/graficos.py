#!/usr/bin/env python
# coding: utf-8

# Configuracion inicial
import pandas  # libreria para procesamiento de datos muy usada en la industria
import matplotlib.pyplot as plt  # librería para graficas de python
import json
import seaborn  # Extiende las funcionalidades de matplotlib, muy usado en data visualization

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

    def set_data_from_json(self):
        json_data = self.load_json_file(self.file_path).values()
        self.data = pandas.DataFrame.from_dict(json_data)

    def get_val_from_col(self, col, key):
        result = []
        for row in self.data[col]:
            for field in row:
                if field['name'] == key:
                    result.append(field['value'])
        return result

    def get_val_from_col_and_module(self, col, module, key):
        result = ""
        for row in self.data[col]:
            for field in row:
                if field['module'] == module and field['name'] == key:
                    result = field['value']
        return result


def draw_with_func(sources, draw_func):
    source_length = len(sources)
    for index, source in enumerate(sources):
        data = DfWrapper(source)
        draw_func(data, index, source_length - index)
    plt.legend(loc='best')
    figname = draw_func.__name__ + "-" + \
        "-".join((source.split(".")[0] for source in sources))
    print(f"Saving fig to {figname}.png")
    plt.savefig(figname)
    plt.close()


def draw_delay(data, index, rindex):
    delay = data.get_val_from_col_and_module(
        "scalars", "Network.node[5].app", "Average delay")
    name = f"{data.file_path.split('.json')[0]}"
    plt.bar([name], delay, ec="black", lw=2)


def draw_delays(sources):
    plt.title("Delays")
    plt.ylabel("Average Delay")
    draw_with_func(sources, draw_delay)


def draw_hop(data, index, rindex):
    hops = data.get_val_from_col_and_module(
        "scalars", "Network.node[5].app", "hopCount")
    name = f"{data.file_path.split('.json')[0]}"
    plt.bar([name], hops, ec="black", lw=2)


def draw_hops(sources):
    plt.title("Saltos")
    plt.ylabel("Average Hops")
    draw_with_func(sources, draw_hop)


def main():
    plt.style.use("ggplot")
    caso1 = [
        "p1c1.json",
        "p2c1.json",
    ]
    caso2 = [
        "p1c2_IAT4.json",
        "p2c2_IAT4.json",
    ]
    draw_delays(caso1)
    draw_delays(caso2)
    draw_hops(caso1)
    draw_hops(caso2)


if __name__ == '__main__':
    main()
