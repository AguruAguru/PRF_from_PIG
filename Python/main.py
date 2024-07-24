import math
import sys
import typing
import matplotlib.pyplot as plt
sys.set_int_max_str_digits(10000000)

############# Testing ###############

from random import randint
def random_succient_output(n, out_len, tqdm, run_time):
    encoded = "".join([str(randint(0,1)) for i in range(n)])
    T = TM(encoded)
    res = ""
    bucket = out_len(n)//100
    for i in tqdm(range(out_len(n))):
        if i%bucket == 0:
            # print(i//bucket)
            pass
        curr = T.run("{0:b}".format(i), run_time(n))
        if len(curr):
            res += curr[0]
        else:
            res += "0"
    return res

def ternary_to_hex_local(s):
    return "{0:x}".format(int(s,3))

def ternary_to_bin_local(s):
    return "{0:b}".format(int(s,3))

import os
import zlib
import gzip
import bz2
import lzma
# import lz4.frame
# import zstandard as zstd

results = []

def generate_binary_data(size):
    """Generate binary data."""
    s = ternary_to_hex_local(random_succient_output(192, out_len=lambda n: n**2, run_time=lambda n: n**1.3))    
    if len(s)%2:
        s = "0" + s    
    b = bytes.fromhex(s)
    return b

def compress_and_measure(data):
    """Compress data using different algorithms and measure sizes."""
    compressed_sizes = {}

    zlib_data = zlib.compress(data)
    compressed_sizes['zlib'] = len(zlib_data)

    # gzip compression
    with gzip.open('temp.gz', 'wb') as f:
        f.write(data)
    gzip_size = os.path.getsize('temp.gz')
    os.remove('temp.gz')
    compressed_sizes['gzip'] = gzip_size

    # bz2 compression
    bz2_data = bz2.compress(data)
    compressed_sizes['bz2'] = len(bz2_data)

    # lzma compression
    lzma_data = lzma.compress(data)
    compressed_sizes['lzma'] = len(lzma_data)

    # lz4 compression
    # lz4_data = lz4.frame.compress(data)
    # compressed_sizes['lz4'] = len(lz4_data)

    # zstd compression
    # zstd_data = zstd.compress(data)
    # compressed_sizes['zstd'] = len(zstd_data)
    return compressed_sizes

def test_compression(data_size):
    """Test compression on binary data."""
    binary_data = generate_binary_data(data_size)
    original_size = len(binary_data)
    # print(f"Original size: {original_size} bytes")

    compressed_sizes = compress_and_measure(binary_data)
    if original_size != 7304:
        a = 1/0
    results.append((original_size, compressed_sizes, binary_data))

def compression_test_main():
    data_size = 10000  # Adjust this to the size of your binary data
    for i in range(3000):
        # test_compression(data_size)
        try:
            test_compression(data_size)
        except:
            pass
            # print("l")
    print("cases: {}".format(len(results)))
    print("original size: {}".format(results[0][0]))
    avg_dist = 0
    cnt = 0
    best_compression = float('inf')
    avg_comp = [0,0,0,0]
    comps = []
    for i in range(len(results)):
        best_compression = min([best_compression] + list(results[i][1].values()))
        for k in range(len(avg_comp)):
            avg_comp[k] += list(results[i][1].values())[k]
        
        for j in range(i+1, len(results)):
            avg_dist += sum([int.bit_count(a ^ b) for a, b in zip(results[i][2], results[j][2])])/8
            cnt += 1
    print("avg hamming distance: {}".format(avg_dist/cnt))
    print("best compression: {}".format(best_compression))
    print("avg comps: {} {} {}".format(avg_comp[0]/len(results), avg_comp[1]/len(results), avg_comp[2]/len(results)))

######################################## 

############## Construction ############
alphabet = ["0", "1", "2", "B"]

class Quintuple:
    def __init__(self):
        self.init_state = None
        self.read = None
        self.write = None
        self.move_dir = None
        self.next_state = None

    def __repr__(self):
        return f"init_state = q{self.init_state}, read = {self.read}, write = {self.write}, move_dir = {self.move_dir}, next_state = q{self.next_state}"

def get_next_chunk(bitstring):
    curr = 0
    last = bitstring[0]
    for bit in bitstring:
        if bit == last:
            curr += 1
        else:
            last = bit
            yield curr
            curr = 1
    if curr:
        yield curr

class TM:
    def __init__(self, bitstring = None):
        self.transitions = []
        self.log_running = None
        self.max_position_returned_to_zero = 0

        if bitstring:
            self.decode_bitstring_smart(bitstring)
    
    def start_tracking_running(self):
        self.log_running = dict()
        self.max_position_returned_to_zero = 0

    def get_states_count(self, l):
        states = 1
        while True:
            if states * 4 * (3 + math.ceil(math.log2(states))) > l:
                return states - 1
            states += 1

    def decode_bitstring_smart(self, encoded_tm):
        index = 0
        states = self.get_states_count(len(encoded_tm))
        # print(states, states * 4 * (3 + math.ceil(math.log2(states))), len(encoded_tm))
        for i in range(states * 4):
            q = Quintuple()

            q.init_state = i//4

            q.read = alphabet[i % 4]

            q.write = alphabet[int(encoded_tm[index:index+2], 2)]

            q.move_dir = ["R", "L"][int(encoded_tm[index+2])]

            q.next_state = int(encoded_tm[index+3:index + 3 + math.ceil(math.log2(states))], 2)

            index += 3 + math.ceil(math.log2(states))

            self.transitions.append(q)


    def decode_bitstring(self, encoded_tm):
        gen = get_next_chunk(encoded_tm)
        inputs = 1

        last = encoded_tm[0]
        for c in encoded_tm:
            if c != last:
                inputs += 1
                last = c
        # print(inputs, (inputs // 5) * 5)

        for i in range(inputs // 5):
            q = Quintuple()

            q.init_state = next(gen)

            q.read = alphabet[(next(gen) - 1) % len(alphabet)]

            q.write = alphabet[(next(gen) - 1) % len(alphabet)]

            q.move_dir = ["R", "L"][(next(gen) - 1) % 2]

            q.next_state = next(gen)

            self.transitions.append(q)

    def run(self, input_str, running_time = 10000):
        tape = list(input_str)
        tape_position = 0
        time = 0
        state = self.transitions[0].init_state
        offset = 0
        max_position = 0

        if self.log_running != None:
            self.log_running[int(input_str, 2)] = []

        # while state != self.transitions[-1].next_state and time < running_time:
        # while time < running_time:
        #     time += 1
        for time in range(running_time):

            if 0 > tape_position:
                tape = ["B"] + tape
                tape_position = 0
                offset += 1

            if self.log_running:
                self.log_running[int(input_str, 2)].append(tape_position - offset)
                max_position = max(max_position, tape_position - offset)
                if not (tape_position - offset):
                    self.max_position_returned_to_zero = max(max_position, self.max_position_returned_to_zero)

            if tape_position >= len(tape):
                tape.append("B")

            current_symbol = tape[tape_position]
            current_transition = None

            for transition in self.transitions:
                if transition.init_state == state and transition.read == current_symbol:
                    current_transition = transition
                    break

            if current_transition:
                tape[tape_position] = current_transition.write
                if current_transition.move_dir == "L":
                    tape_position -= 1
                elif current_transition.move_dir == "R":
                    tape_position += 1

                state = current_transition.next_state
            else:
                print("INVALID", transition)
                break        
        # return "".join(tape).replace("B", "")
        self.last_state = state
        return "".join(tape[:offset]).replace("B", "") + "#" + "".join(tape[offset:]).replace("B", "")

    def plot_running_data_slice(self, inp, ax = None):
        time = len(list(self.log_running.values())[0])
        X = [i for i in range(time)]
        Y = [self.log_running[inp][t] for t in X]
        if ax == None:
            plt.plot(X,Y)
            plt.show()
        else:
            ax.plot(X,Y)        

    def plot_running_data(self):        
        from mpl_toolkits.mplot3d import Axes3D
        
        time = len(list(self.log_running.values())[0])
        points = []
        for inp in range(len(self.log_running.keys())):
            for t in range(time):
                    points.append((inp,t,self.log_running[inp][t])) 

        fig = plt.figure()
        # ax = Axes3D(fig)
        ax = fig.add_subplot(111, projection='3d')

        # plot_geeks = ax.scatter(
        #     [p[0] for p in points], 
        #     [p[1] for p in points], 
        #     [p[2] for p in points], color='blue')
        import numpy as np
        from matplotlib import cm

        X = np.array([inp for inp in range(len(self.log_running.keys()))])
        Y = np.array([t for t in range(time)])
        Z = np.array([[self.log_running[inp][t] for t in Y] for inp in X])
        
        my_col = cm.jet(np.random.rand(Z.shape[0],Z.shape[1]))
        
        ax.plot_surface(X, Y, Z, rstride=1, cstride=1, facecolors = my_col,
        linewidth=0, antialiased=False)
        ax.set_title("TM running log")

        ax.set_xlabel('Input')
        ax.set_ylabel('Time')
        ax.set_zlabel('Head Location')
        
        # displaying the plot
        plt.show()
def tmp(m,d):
    i = 1
    while i*math.log2(i) < m/d + 0.0001:
        i += 1
    return i - 1


class Automata:
    def __init__(self, d, enc):
        self.d = d
        self.n = n = tmp(len(enc),d)
        self.vertices = [i for i in range(n)]
        self.edges = [[] for i in range(n)]

        for i in range(n*(d//2)):
            t = int(enc[int(math.log2(n))*i:int(math.log2(n))*(i+1)], 2)
            self.edges[i//(d//2)].append(t)
            self.edges[t].append(i//(d//2))
        
        # for edge in self.edges:
        #     print(len(edge))
    
    def run(self, input_str):
        ind = int(math.log2(self.n))
        s = int(input_str[:ind], 2)
        for i in range(ind, len(input_str), int(math.log2(self.d))):
            s = self.edges[s][int(input_str[i:i + int(math.log2(self.d))], 2) % (len(self.edges[s]))]
        return str(s%2)            





import galois
class DesignsPolynomials:
    l: int
    q: int
    d: int # universe
    m: int
    log_q: int
    I: typing.Dict[int,typing.List[int]]

    def __init__(self, l, m):
        self.l = l
        self.m = m
        self.log_q = (l-1).bit_length() 
        self.q = 1<<self.log_q
        self.d = l*self.q
        self.I = {}

    def explicit_calculation(self, i, y):
        # index in the design and the string to operate on

        if i not in self.I:
            GF_q = galois.GF(self.q)  # the finite field F_q
            
            powers = range(i.bit_length() // self.log_q + (i.bit_length() % self.log_q != 0))
            coeffs = [(i >> self.log_q * j) % self.q for j in powers][::-1]  # i in base q, determines the coeffs for the polynomial

            poly = galois.Poly(coeffs, field=GF_q)
            # print(poly)
            I_i = sorted([
                k*self.q + int(poly(k))
                for k in range(self.l)
            ]) # the i'th design in the designs collection

            self.I[i] = I_i

        # print(I_i)

        result = ""
        for index in range(self.l):
            result += y[self.I[i][index]]
        
        return result

        
class NW:
    def __init__(self, hard_function, security_param, n, designs=None):
        self.hard_function = hard_function        
        self.log_security_param = math.ceil(math.log2(security_param))  # TODO in which direction to round
        if not designs:
            self.designs = DesignsPolynomials(
                l = math.ceil(2 * self.log_security_param), #TODO incorrect value
                m = math.ceil(n ** (1/8))
            )
        else:
            self.designs = designs
    
    def explicit_calculation(self, i, y):
        y_restricted = self.designs.explicit_calculation(i,y)
        return self.hard_function(y_restricted)
    
    def restrict_y(self, i, y):
        return self.designs.explicit_calculation(i,y)

def hard_function_cand(pi, n):
    T = TM(pi[:pi.rfind("1")])
    def func(input_str):
        result = T.run(input_str, running_time=n//10)
        return str(T.last_state % 2)
        # if len(result):
        #     return result[0]
        # else:
        #     return "0"
    return func

def designs_for_NW(security_param, n):
    log_security_param = math.ceil(math.log2(security_param))  # TODO in which direction to round
    designs = DesignsPolynomials(
        l = math.ceil(2 * log_security_param), #TODO incorrect value
        m = math.ceil(n ** (1/8))
    )
    return designs

def h(pis, y, i, security_param, n):
    designs = designs_for_NW(security_param, n)
    curr = 0
    for j in range(len(pis)):
        curr_NW = NW(
            hard_function=hard_function_cand(pis[j], n),
            security_param=security_param,
            n=n,
            designs=designs)
        curr = (curr + int(curr_NW.explicit_calculation(i, y[j])))%3

    return str(curr)

def h_expand(args):
    return h(*args)

from tqdm import tqdm
def handle_range_for_async(start, end, pi_primes, ys, security_param, n):
    if not start:
        start = 1
    res = ["X" for i in range(start, end)]
    print("here")
    for j in tqdm(range(start,end)):
        res[j-start] = h(pi_primes, ys, j, security_param, n)
        # progbar.update(1)    
    return "".join(res)