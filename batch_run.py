#!/usr/bin/env python3

import click
import subprocess
import json
import os
import time
import logging
import traceback
import multiprocessing
from multiprocessing import Queue


logging.basicConfig(
    format='%(asctime)s %(levelname)-8s %(message)s',
    level=logging.INFO,
    datefmt='%Y-%m-%d %H:%M:%S')
log = logging.getLogger()

@click.group()
def main():
    pass


def get_player_config(port):
    return {
        "Tcp": {
            "host": None,
            "port": port,
            "accept_timeout": None,
            "timeout": None,
            "token": None
        }
    }


def worker(args):
    idx, p1, p2, lr_bin, start_seed, level, nthreads, count, queue = args
    port1 = 32003 + idx * 2
    port2 = port1 + 1
    port3 = port2 + 1
    port4 = port3 + 1
    seed = start_seed + idx
    # return
    config = {
        "game": {
            "Create": "Finals"
        },
        "players": [
            get_player_config(port1),
            get_player_config(port2),
            # get_player_config(port3),
            # get_player_config(port4)
        ],
        "seed": seed
    }

    cwd = os.getcwd()

    config_path = os.path.join(cwd, f"tmp/_config{idx}.json")
    result_path = os.path.join(cwd, f"tmp/_result{idx}.txt")
    with open(config_path, "w") as out:
        json.dump(config, out, indent=4)
    if os.path.exists(result_path):
        os.remove(result_path)

    try:
        with subprocess.Popen(f"{lr_bin} --batch-mode --config {config_path} --save-results {result_path} --log-level warn".split(" ")) as process:
            time.sleep(1.0)
            subprocess.Popen([p1, "127.0.0.1", str(port1), "0000000000000000"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            subprocess.Popen([p2, "127.0.0.1", str(port2), "0000000000000000"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            # subprocess.Popen([p2, "127.0.0.1", str(port3), "0000000000000000"], stdout=subprocess.DEVNULL)
            # subprocess.Popen([p2, "127.0.0.1", str(port4), "0000000000000000"], stdout=subprocess.DEVNULL)
            process.wait()
            if os.path.exists(result_path):
                with open(result_path) as result_inp:
                    result = json.load(result_inp)
                    log.info("game with seed=%d results: %d - %d", seed, result["results"][0], result["results"][1])
                    #print(" - ".join([("CRASHERD " if result["players"][i]["crashed"] else "") + str(result["results"][i]) for i in range(2)]))

                    queue.put(result["results"])

    except Exception:
        log.error(traceback.format_exc())
    finally:
        pass


def start_process():
    pass


@main.command()
@click.option('--p1', type=str, required=True)
@click.option('--p2', type=str, required=True)
@click.option('--lr-bin', type=str, default="../aicup2020-macos/aicup2020")
@click.option('--seed', type=int, default=1)
@click.option('--level', type=str, default="Simple")
@click.option('--nthreads', type=int, default=4)
@click.option('--count', type=int, default=1)
def run(p1, p2, lr_bin, seed, level, nthreads, count):
    m = multiprocessing.Manager()
    queue = m.Queue()
    if not os.path.exists("tmp"):
        os.makedirs("tmp")

    pool = multiprocessing.Pool(processes=nthreads, initializer=start_process)
    pool.map(worker, [(
        i,
        p1, p2, lr_bin, seed, level, nthreads, count, queue
    ) for i in range(count)])

    p1_win = 0
    p2_win = 0
    draws = 0
    while not queue.empty():
        res = queue.get()
        if len(res) == 2:
            p1_result, p2_result = res
        else:
            p1_result, p2_result, _, _ = res
        if p1_result > p2_result:
            p1_win += 1
        elif p1_result < p2_result:
            p2_win += 1
        else:
            draws += 1

    log.info(f"p1_win: {p1_win}, p2_win: {p2_win}, draws: {draws}")

if __name__ == "__main__":
    main()