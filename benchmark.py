import ctypes
import os
import datetime

class cBenchCounters(ctypes.Structure):
    '''
    This has to match the returned struct in library.c
    '''
    _fields_ = [ 
                ("attempted_removes", ctypes.c_int),
                ("successful_removes", ctypes.c_int),
                ("attempted_steals", ctypes.c_int),
                ("successful_steals", ctypes.c_int),
                ("items_added", ctypes.c_int)
               ]

class cBenchResult(ctypes.Structure):
    '''
    This has to match the returned struct in library.c
    '''
    _fields_ = [ ("time", ctypes.c_float),
                 ("reduced_counters", cBenchCounters) ]

class Benchmark:
    '''
    Class representing a benchmark. It assumes any benchmark sweeps over some
    parameter xrange using the fixed set of inputs for every point. It provides
    two ways of averaging over the given amount of repetitions:
    - represent everything in a boxplot, or
    - average over the results.
    '''
    def __init__(self, bench_function, parameters,
                 repetitions_per_point, xrange, basedir, name):
        self.bench_function = bench_function
        self.parameters = parameters
        self.repetitions_per_point = repetitions_per_point
        self.xrange = xrange
        self.basedir = basedir
        self.name = name

        self.data = {}
        self.succ_data = {}
        self.now = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S")

    def run(self):
        '''
        Runs the benchmark with the given parameters. Collects
        repetitions_per_point data points and writes them back to the data
        dictionary to be processed later.
        '''
        t = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S")
        print(f"Starting Benchmark {self.name} run at {t}")

        for x in self.xrange:
            tmp1 = [] # time per benchmark
            tmp2 = [] # successful operations per second
            for r in range(0, self.repetitions_per_point):
                results = self.bench_function( x, *self.parameters )
                result = results.time*1000
                tmp1.append( result )
                succ = results.reduced_counters.successful_removes+results.reduced_counters.successful_steals+results.reduced_counters.items_added
                tmp2.append( succ/result )
            self.data[x] = tmp1
            self.succ_data[x] = tmp2

    def write_avg_data(self):
        '''
        Writes averages for each point measured into a dataset in the data
        folder timestamped when the run was started.
        '''
        if self.now is None:
            raise Exception("Benchmark was not run. Run before writing data.")

        print(f"Saving data to {self.basedir}/data/{self.now}/avg/{self.name}_time.data")

        try:
            os.makedirs(f"{self.basedir}/data/{self.now}/avg")
        except FileExistsError:
            pass
        with open(f"{self.basedir}/data/{self.now}/avg/{self.name}_time.data", "w")\
                as datafile:
            datafile.write(f"x datapoint\n")
            for x, box in self.data.items():
                datafile.write(f"{x} {sum(box)/len(box)}\n")
        with open(f"{self.basedir}/data/{self.now}/avg/{self.name}_success.data", "w")\
                as datafile:
            datafile.write(f"x datapoint\n")
            for x, box in self.succ_data.items():
                datafile.write(f"{x} {sum(box)/len(box)}\n")

def benchmark():
    '''
    Requires the binary to also be present as a shared library.
    '''
    basedir = os.path.dirname(os.path.abspath(__file__))
    binary = ctypes.CDLL( f"{basedir}/build/library.so" )
    # Set the result type for each benchmark function
    binary.small_bench.restype = cBenchResult

    # The number of threads. This is the x-axis in the benchmark, i.e., the
    # parameter that is 'sweeped' over.
    num_threads = [2,4,8,16]#,32,64,128,256]

    # Parameters for the benchmark are passed in a tuple, here (1000,). To pass
    # just one parameter, we cannot write (1000) because that would not parse
    # as a tuple, instead python understands a trailing comma as a tuple with
    # just one entry.
    elements = 100000
    # smallbench_single_producer = Benchmark(binary.small_bench, (0, elements), 3, num_threads, basedir, "single_producer")
    # smallbench_single_consumer = Benchmark(binary.small_bench, (1, elements), 3, num_threads, basedir, "single_consumer")
    smallbench_50_50 = Benchmark(binary.small_bench, (2, elements), 3, num_threads, basedir, "split_50_50")
    smallbench_produce_and_consume = Benchmark(binary.small_bench, (3, elements), 3, num_threads, basedir, "produce_and_consume")

    # smallbench_single_producer.run()
    # smallbench_single_producer.write_avg_data()
    # smallbench_single_consumer.run()
    # smallbench_single_consumer.write_avg_data()
    smallbench_50_50.run()
    smallbench_50_50.write_avg_data()
    smallbench_produce_and_consume.run()
    smallbench_produce_and_consume.write_avg_data()


if __name__ == "__main__":
    benchmark()
