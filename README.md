# GBC-Project
Group_betweeness_Centralty on multithreaded CPU using C++
This is my B.Sc Final Year Project.


**Fast Parallel Group Betweenness Centrality Calculation in Complex Network** 
# **I. INTRODUCTION** 
Complex networks are used to study the structure and dynamics of complex systems in various disciplines. For example, social networks, protein interaction networks, and computer networks such as the Internet are all classified as complex networks. In social networks, vertices are usually individuals, and edges characterize the relations between them; in computer networks, vertices might be routers connected to each other through communication lines. 

In the analysis of complex networks, the evolution of the importance of vertices and edges is widely used. To evaluate the importance various centrality measures have been suggested.  Betweenness centrality (BC) is considered to be a good approximation for the quantity of information passing through a vertex in communication networks. Group Betweenness Centrality (GBC), a natural extension of the Betweenness measures was defined by Everett and Borgatti. The GBC can be used to estimate the influence of a group of vertices over the information flow in the network. 

Now the calculation of BC of all nodes of a complex network is computationally expensive. Until the study of Brandes, algorithms computing BC off all vertices of a network had a running time complexity of O(n<sup>3</sup>). Brandes proposed an algorithm for computing BC with O(nm) and O(nm+n<sup>2</sup> log n) time complexity and O(n+m) space complexity for unweighted and weighted networks, respectively. 

The problem of finding a group of vertices of a given size g in a network that has the highest GBC is in general an NP-hard problem. Brandes and Puzis et al. proposed algorithms to compute the GBC of any given group of vertices. The latter algorithm can compute the GBC of a given group of vertices within reasonable times even for large-scale networks. 


# **II. Implementation** 

![Screenshot 2023-10-13 083515](https://github.com/Aniket10singh16/GBC-Project/assets/37691313/becd27d7-c6db-4607-8e98-898f99e39029)

## **B. Understanding the Algorithm** 
The algorithm enclosed inside the first loop describes the calculation of Group Betweenness Centrality with respect to just one vertex which may be just a small portion of the whole calculation. In order to calculate the whole GBC, the algorithm needs to run for each and every vertex of the graph. Even though the algorithm proposed by Brandes is a significant improvement over the previous adaptation but still running the algorithm for each and every vertex serially will impose an enormous amount of time to calculate. Thus, to improve this situation, parallel running of the algorithm for each vertex on each thread of the Operating System is necessary and implemented to reduce the computation time significantly.  



The forward phase of the algorithm is basically used to calculate the distance of each node from the starting node or if other nodes are reachable from the starting node, and also to calculate the number of shortest paths from the starting node to all other nodes. Breath First Search is being used to calculate the Shortest Paths and to count and store the number of shortest paths available for each vertex from the starting vertex. BFS is being used rather than DFS or Dijkstra’s Algorithm as DFS doesn’t guarantee the shortest path and Dijkstra not being able to implement each vertex separately. Before the BFS starts it is assumed that only the starting node has been visited and no nodes have been completely discovered. BFS starts by discovering all the adjacent nodes of the starting node and adds it to the visited category, as BFS used QUEUE data structure, the discovering of the vertex happens in First in First Out fashion, during this process each vertex visited is given a level number accordingly and also the number of shortest paths is calculated. In this phase, only all the predecessors of each vertex are checked and added to a list for later use. A predecessor vertex is a vertex that has an edge connecting the later node to the formal node and having a level number 1 less than the vertex being checked. 



The Backward Phase of the algorithm calculates the dependency of each vertex. Brandes observed that the accumulated dependency values can be computed (given in the algorithm). The Backward phase runs until the stack becomes empty, the stack contains each and every node visited accordingly and is accumulated in the forward phase. For every node in the stack, their predecessors are checked if they belong to the Group to be checked or not, and accordingly, the dependency of that node is calculated and stored. After the calculation of dependency is checked for every vertex, only those calculated dependency of a node is added to the  𝑐𝑐<sub>𝐺𝐺𝐵𝐵</sub>(𝐶𝐶) which belongs to the group and does not include the starting node. 



It is to be reminded again that the Forward and Backward phases are run for just one vertex and similarly the same steps are taken for all other vertex parallelly to calculate the final GBC. 


# **III. Performance Evaluation** 
First, we tested the performance uplift from calculating GBC serially to computing GBC parallelly on CPU threads. Here we used four network graphs to test our Algorithm performance- As we can see from **TABLE 1**, for Dolphins and football network graph the performance gain from serial execution to parallel execution is not significant but as the graph size increases the change is more apparent. In Facebook network graph with around 4000 nodes with parallel processing a total of **70.68%** time decreases in computing GBC over serial execution, that is over 3 times the performance uplift. 





|S.no |Name |No of Vertex |No of edges |Serial |Parallel |
| :- | :-: | :- | :- | :-: | :-: |
|1 |Dolphins.txt |62 |318 |0.0032174s |0.0025614s |
|2 |Football.txt |115 |1,232 |0.0164311s |0.0169048s |
|3 |Facebook.txt |4,039 |1,76,468 |12.7161s |3.72777s |
|4 |Wik.txt |7118 |2,07,234 |65.0384s |27.5983s |
## **Table 1: Comparison between serial computation of GBC with Parallel Computation**  
** 

<table><tr><th valign="top">S.no </th><th valign="top">Name </th><th valign="top">No of Vertex </th><th valign="top">No of Edges </th><th valign="top">Group Size </th><th valign="top">Time Taken </th></tr>
<tr><td valign="top">1 </td><td rowspan="4">Wiki.txt </td><td rowspan="4">7118 </td><td rowspan="4">2,07,234 </td><td valign="top">10 </td><td valign="top">27.5983s </td></tr>
<tr><td valign="top">2 </td><td valign="top">100 </td><td valign="top">39.5594s </td></tr>
<tr><td valign="top">3 </td><td valign="top">1000 </td><td valign="top">39.6422s </td></tr>
<tr><td valign="top">4 </td><td valign="top">5000 </td><td valign="top">28.7005s </td></tr>
</table>
## **Table 2: Comparison of how group size impacts computation time** 
** 

<table><tr><th>S.no </th><th>Name </th><th>No of Vertex </th><th>No of Edges </th><th>Group size </th><th>Time Taken </th></tr>
<tr><td>1 </td><td>Dolphins.txt </td><td>62 </td><td>318 </td><td rowspan="4">10 </td><td>0.0025614s </td></tr>
<tr><td>2 </td><td>Football.txt </td><td>115 </td><td>1,232 </td><td>0.0169048s </td></tr>
<tr><td>3 </td><td>Facebook.txt </td><td>4,039 </td><td>1,76,468 </td><td>3.72777s </td></tr>
<tr><td>4 </td><td>Wik.txt </td><td>7118 </td><td>2,07,234 </td><td>27.5983s </td></tr>
</table>
## **Table 3: Computation time taken by different sizes of Network** 






<table><tr><th>S.no </th><th>Name </th><th>No of Vertex </th><th>No of Edges </th><th valign="top"><p>No of </p><p>Threads </p></th><th>Time Taken </th></tr>
<tr><td valign="top">1 </td><td rowspan="4">Facebook.txt </td><td rowspan="4">4,039 </td><td rowspan="4">1,76,468 </td><td valign="top">2 </td><td valign="top">7.14432s </td></tr>
<tr><td valign="top">2 </td><td valign="top">4 </td><td valign="top">7.28984s </td></tr>
<tr><td valign="top">3 </td><td valign="top">6 </td><td valign="top">6.99855s </td></tr>
<tr><td valign="top">4 </td><td valign="top">8 </td><td valign="top">3.54981s </td></tr>
</table>
## **Table 4: How Computational time changes with the number of threads used to calculate GBC** 


Note: Execution Time can vastly vary from time to time due to various reasons, for example, background processes, OS scheduling, memory available, etc. The Time Taken mentioned above are just approximate. 

# **IV. Conclusion** 

From the above results, it is evident that parallel computing alongside the use of a suitable data structure from which data can be stored and accessed in a reasonable time, the overall time required to compute the group betweenness centrality. Although this system is not consistent the major factor being the number of concurrent threads a given system can handle, the amount of system memory, background processes running on the system along with their priority, and also how efficient the operating system's scheduling is being used.

It should also be noted that the time taken to compute different group sizes of the same data file doesn't vary much as the algorithm processes through every vertex regardless of the group size but only acknowledges those vertices that are present in the search group.

As far as the scope of the given process is concerned, the performance of this system needs to be tested with much larger datasets using a better system having many more threads. Future rework of the proposed system may also be required.

