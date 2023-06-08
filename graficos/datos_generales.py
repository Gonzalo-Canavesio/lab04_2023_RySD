from utilizacion import get_total_packets_by_key
from graficos import DfWrapper


def get_general_data(source):
    data = DfWrapper(source)
    paquetes_emisor = get_total_packets_by_key(data, "sentPackets")[0]
    paquetes_receptor = get_total_packets_by_key(data, "receivedPackets")[0]
    delay = data.get_val_from_col_and_module(
        "scalars", "Network.node[5].app", "Average delay")
    hops = data.get_val_from_col_and_module(
        "scalars", "Network.node[5].app", "hopCount")
    print(f"{source}\n\n{paquetes_emisor=}\n{paquetes_receptor=}\n{delay=}\n{hops=}")
    print("-" * 50)


def main():
    sources = [
        "p1c1.json",
        "p1c2_IAT4.json",
        "p2c1.json",
        "p2c2_IAT4.json",
    ]
    for source in sources:
        get_general_data(source)


if __name__ == '__main__':
    main()
