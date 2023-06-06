from graficos import DfWrapper, NODE_AMMOUNT
import matplotlib.pyplot as plt  # librería para graficas de python
import pandas


class DfWrapperCarga(DfWrapper):
    def get_inter_arrival_times(self):
        result = [
            self.get_inter_arrival_time(row) for _, row in self.data.iterrows()]
        print(result)
        return result

    def get_inter_arrival_time(self, item):
        value = item['config'][4]["Network.node[{0,1,2,3,4,6,7}].app.interArrivalTime"]
        return float(value.split("(")[-1][:-1])

    def sort_json(self, data):
        data = list(data)
        return sorted(data, key=self.get_inter_arrival_time)

    def set_data_from_json(self):
        json_data = self.load_json_file(self.file_path).values()
        sorted_data = self.sort_json(json_data)
        self.data = pandas.DataFrame.from_dict(sorted_data)


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
    plt.plot(arriv_times, carga, linewidth=1 + rindex*1.5, zorder=index,
             label=f"{data.file_path.split('.json')[0]}")


def draw_with_func(sources, draw_func):
    source_length = len(sources)
    for index, source in enumerate(sources):
        data = DfWrapper(source)
        draw_carga(data, index, source_length - index)
    plt.legend(loc='best')
    plt.show()


def main():
    plt.style.use("ggplot")
    sources = [
        "p1c2.json",
        "p2c2.json",
    ]
    source_length = len(sources)
    plt.xlabel("interArrivalTime")
    plt.ylabel("Recibidos/Enviados")
    for index, source in enumerate(sources):
        data = DfWrapperCarga(source)
        draw_carga(data, index, source_length - index)
    plt.legend(loc='best')
    plt.show()


if __name__ == '__main__':
    main()
