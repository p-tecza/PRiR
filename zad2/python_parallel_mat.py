import threading

#how many threads
N = 4

class myThread (threading.Thread):
   def __init__(self, s_index, e_index, n, m, it,thread_number):
      threading.Thread.__init__(self)
      self.s_index = s_index
      self.e_index = e_index
      self.n = n
      self.m = m
      self.it = it
      self.thread_number = thread_number
   def run(self):
      print ("Starting thread nr ",self.thread_number)
      calculate(self.s_index,self.e_index,self.n,self.m,self.it)
      print ("Exiting thread nr ",self.thread_number)

lock=threading.Lock()
global_sum=0
fa = open("A.txt","r")
fb = open("B.txt","r")
ma = int(fa.readline())
na = int(fa.readline())
mb = int(fb.readline())
nb = int(fb.readline())

A=[[0]*na for i in range(0,ma)]
B=[[0]*nb for i in range(0,mb)]

if(na!=mb):
    print("An musi byc rowne Bm!\n")
    exit()

it_matrix = na

C=[[0]*nb for i in range(0,ma)]

A_file_vals = fa.read()
B_file_vals = fb.read()
A_vals=[]
B_vals=[]

for word in A_file_vals.split():
    A_vals.append(float(word))

for word in B_file_vals.split():
    B_vals.append(float(word))


it=0
for i in range(0,ma):
    for j in range(0,na):
        A[i][j] = A_vals[it]
        it+=1

it=0
for i in range(0,mb):
    for j in range(0,nb):
        B[i][j] = B_vals[it]
        it+=1


trivial_mul_amount = ma*nb

if trivial_mul_amount < N:
    print("Zbyt duzo watkow do rownoleglych obliczen!")
    print("Ustawiam na rownowartosc ilosci sum czastkowych")
    N=trivial_mul_amount

values_per_thread = int(trivial_mul_amount / N)
remainder = trivial_mul_amount % N

indexes = [0 for i in range(0,N+1)]
indexes[0]=0

for i in range(1,N+1):
    indexes[i]=values_per_thread*i

new_it=0
while(remainder>0):
    for i in range(N-new_it,N+1):
        indexes[i]+=1
    remainder-=1


def calculate(s_index,e_index,n, m,it):
    local_sum = 0
    for i in range(s_index,e_index):
        m_result_index = int(i / n)
        n_result_index = int(i % n)
        for j in range(0,it):
            local_sum = local_sum + A[m_result_index][j]*B[j][n_result_index]
        global global_sum
        lock.acquire()
        global_sum+=local_sum
        lock.release()
        C[m_result_index][n_result_index]=local_sum; 
        local_sum=0

threads=[]
for i in range(0,N):
    tr = myThread(indexes[i],indexes[i+1],nb,ma,it_matrix,i+1)
    tr.start()
    threads.append(tr)
    

for tr in threads:
    tr.join()

print("C:")
for row in C:
    print(row)
print("Suma globalna: ",global_sum)
