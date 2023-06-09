import ctypes
import os
import datetime
import sys
import getopt

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

class cLockBenchResult(ctypes.Structure):
    _fields_ = [ ("time", ctypes.c_float) ]

class Benchmark:
    '''
    Class representing a benchmark. It assumes any benchmark sweeps over some
    parameter xrange using the fixed set of inputs for every point. It provides
    two ways of averaging over the given amount of repetitions:
    - represent everything in a boxplot, or
    - average over the results.
    '''
    def __init__(self, bench_function, parameters,
                 repetitions_per_point, xrange, basedir, name, lock_based = False):
        self.bench_function = bench_function
        self.parameters = parameters
        self.repetitions_per_point = repetitions_per_point
        self.xrange = xrange
        self.basedir = basedir
        self.name = name
        self.lock_based = lock_based
        self.data = {}
        self.succ_data = {}

    def run(self):
        '''
        Runs the benchmark with the given parameters. Collects
        repetitions_per_point data points and writes them back to the data
        dictionary to be processed later.
        '''
        t = datetime.datetime.now()
        tstr = t.strftime("%Y-%m-%dT%H:%M:%S")
        print(f"Starting Benchmark {self.name} run at {tstr}")
        self.now = t
        
        for x in self.xrange:
            tmp1 = [] # time per benchmark
            tmp2 = [] # successful operations per second
            for r in range(0, self.repetitions_per_point):
                results = self.bench_function( x, *self.parameters )
                result = results.time*1000

                
                print("-------- Results " + str(x) + "----------")
                print("attempted_removes: " + str(results.reduced_counters.attempted_removes))
                print("items_added: " + str(results.reduced_counters.items_added))
                print("successful_steals: " + str(results.reduced_counters.successful_steals))
                print("attempted_steals: " + str(results.reduced_counters.attempted_steals))
                print("successful_removes: " + str(results.reduced_counters.successful_removes))
                print("-------- END Results ----------")
                
                
                



                tmp1.append( result )
                if not self.lock_based:
                    succ = results.reduced_counters.successful_removes+results.reduced_counters.successful_steals+results.reduced_counters.items_added
                    tmp2.append( succ/result )
                    self.succ_data[x] = tmp2
            self.data[x] = tmp1
        t = datetime.datetime.now()-t
        print(f"Benchmark {self.name} took {t}")
            
       

    def write_avg_data(self):
        '''
        Writes averages for each point measured into a dataset in the data
        folder timestamped when the run was started.
        '''
        if self.now is None:
            raise Exception("Benchmark was not run. Run before writing data.")

        print(f"Saving data to {self.basedir}/avg/{self.name}_time.data")

        try:
            os.makedirs(f"{self.basedir}/avg")
        except FileExistsError:
            pass
        with open(f"{self.basedir}/avg/{self.name}_time.data", "w")\
                as datafile:
            datafile.write(f"x datapoint\n")
            for x, box in self.data.items():
                datafile.write(f"{x} {sum(box)/len(box)}\n")

        if not self.lock_based:
            print(f"Saving data to {self.basedir}/avg/{self.name}_success.data")
            with open(f"{self.basedir}/avg/{self.name}_success.data", "w")\
                    as datafile:
                datafile.write(f"x datapoint\n")
                for x, box in self.succ_data.items():
                    datafile.write(f"{x} {sum(box)/len(box)}\n")
        

if __name__ == "__main__":
    opts, args = getopt.getopt(sys.argv[1:], "d:p:i:s:l:e:")
    d = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S") # Directory in /data where to store the results of the benchmark
    p = "split_50_50" # The name of the program to run
    i = 3 # Amount of iterations
    s = "small" # Size of benchmark, either small or big. Determines whether benchmark will run on threads 2-16 or 2-128
    l = False # Lock based. Determines whether to run the lock based version of the selected program
    e = 10000 # Amount of elements to pass on to the program

    programs = {
        "single_producer"       : 0,
        "single_consumer"       : 1,
        "split_50_50"           : 2,
        "produce_and_consume"   : 3
    }

    for opt, arg in opts:
        if opt == "-d":
            d = arg
        if opt == "-p":
            p = arg
        if opt == "-i":
            i = int(arg)
        if opt == "-s":
            s = arg.lower()
        if opt == "-l":
            if arg.lower() == "true":
                l = True
            if arg.lower() == "false":
                l = False
        if opt == "-e":
            e = int(e)
    
    name = p
    if l:
        name = name + "_lock_based"
    small_threads = [2, 4, 8, 16]
    large_threads = small_threads + [32, 64, 128]

    threads = small_threads
    if s == "large":
        threads = large_threads

    basedir = os.path.dirname(os.path.abspath(__file__))
    binary = ctypes.CDLL( f"{basedir}/build/library.so" )
    # Set the result type for each benchmark function
    binary.small_bench.restype = cBenchResult
    binary.small_lock_based_bench.restype = cLockBenchResult

    datadir = basedir + "/data/" + d
    print("-----------------------------------------------------------------------------------")
    print(f"Starting benchmark {name} with {e} elements, {i} iterations, and threads: {threads}")
    print(f"Data will be saved in {datadir}")

    benchmark = Benchmark(binary.small_bench, (programs[p], e), i, threads, datadir, name, l)
    benchmark.run()
    benchmark.write_avg_data()
    print("-----------------------------------------------------------------------------------")

