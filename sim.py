import os
import shutil
import sys
import numpy as np

try:
    CODE_DIR = sys.argv[1]
except Exception as e:
    print(e)
    print("USAGE: python3 sim.py <code-dir> <simulation-name>")
    exit(1)

BIN_NAME = "lab4-kickstarter"

GEN_INTERVALS = [round(num, 2) for num in np.arange(0.1, 15.01, 0.1)]
EXTRA_INI_TEMPLATE = "Network.node[{{0,1,2,3,4,6,7}}].app.interArrivalTime = exponential({})"


def get_result_dirname(sim_name: str):
    return f"{sim_name}-vectors"


def get_vector_data_dirname(gen_interval: float, sim_name: str):
    result_dirname = get_result_dirname(sim_name)
    generation_dirname = f"gen-interval {gen_interval}"
    return os.path.join(result_dirname, generation_dirname)


def get_bin_path():
    return os.path.join(CODE_DIR, BIN_NAME)


def get_file_path(file_name):
    return os.path.join(CODE_DIR, file_name)


def check_file_exists(file_path):
    if not os.path.exists(file_path):
        raise Exception(f"No se encuentra '{file_path}'")


def remove_vector_dir(sim_name):
    results_folder = get_result_dirname(sim_name)
    if os.path.exists(results_folder):
        shutil.rmtree(results_folder)


def create_results_folder(sim_name):
    os.mkdir(get_result_dirname(sim_name))


def run_simulations(extra_ini, omnet_ini, bin_name, sim_name, omnet_output):
    for gen_interval in GEN_INTERVALS:
        write_extra_ini(extra_ini, gen_interval)

        execute_simulation(bin_name, omnet_ini, extra_ini)

        move_results(omnet_output, gen_interval, sim_name)


def write_extra_ini(extra_ini, generation_interval):
    with open(extra_ini, "w") as f_extra_ini:
        text = EXTRA_INI_TEMPLATE.format(generation_interval)
        print(text)
        f_extra_ini.write(
            text
        )


def execute_simulation(bin_name, omnet_ini, extra_ini):
    command = f".{os.sep}{bin_name} -f {omnet_ini} -f {extra_ini} -n {CODE_DIR} -u Cmdenv"
    x = os.system(command + " > /dev/null 2>&1")
    if x != 0:
        raise Exception(f"Error al ejecutar simulación '{bin_name}'")


def move_results(omnet_output, generation_interval, sim_name):
    results_folder = get_vector_data_dirname(generation_interval, sim_name)
    shutil.move(omnet_output, results_folder)


def run_sims(sim_name: str):
    bin_name = get_bin_path()

    omnet_ini = get_file_path("omnetpp.ini")
    extra_ini = get_file_path("extra.ini")

    omnet_output = get_file_path("results")

    check_file_exists(bin_name)
    check_file_exists(omnet_ini)

    remove_vector_dir(sim_name)

    create_results_folder(sim_name)

    run_simulations(extra_ini, omnet_ini, bin_name, sim_name, omnet_output)

    os.remove(extra_ini)


def write_graph_data(sim_name: str):
    json_file = f"{sim_name}.json"
    text = f"opp_scavetool export {get_result_dirname(sim_name)}{os.sep}*{os.sep}*.sca -o {json_file}"
    print(text)
    x = os.system(text)
    assert x == 0

    remove_vector_dir(sim_name)


def main():
    sim_name = sys.argv[2]
    run_sims(sim_name)
    write_graph_data(sim_name)


if __name__ == "__main__":
    main()
