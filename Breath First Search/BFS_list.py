graph = dict()
 
graph['A'] = ['B', 'C']
graph['B'] = ['A', 'D']
graph['C'] = ['A', 'G', 'H', 'I']
graph['D'] = ['B', 'E', 'F']
graph['E'] = ['D']
graph['F'] = ['D']
graph['G'] = ['C']
graph['H'] = ['C']
graph['I'] = ['C', 'J']
graph['J'] = ['I']

#     A
#  B     C
#  D   G H I
# E F      J


## DFS 는 stack
## - 갈 수 있는 곳까지 들어가고 갈곳이 없으면 되돌아 나와 다른길을 찾음
## BFS 는 queue
## - 시작점의 인접한 정점들을 차례로 방문, 방문했던 정점을 다시 시작점으로 해서 다시 인접한 정점들을 차례로 모두 방문

def bfs(graph, start_node):
    # BFS도 마찬가지로 두개의 리스트로 따로 관리
    # BFS는 방문 노드의 인접한 노드를 큐에 저장하는 것이 DFS와 다른 점
    # 1. 방문완료 리스트
    # 2. 방문예정 리스트(need_visit)
    visited, need_visit = list(), list()

    # 처음 노드 설정 -> queue에 삽입
    need_visit.append(start_node)

    # need_visit 에 원소가 존재하면
    while need_visit:
        # queue에 있는 노드를 하나 삭제하고 삭제된 노드는 방문 리스트에서 방문했다고 표시한다.
        ### DFS 랑 다른점이 pop()을 할떼 pop(0)를 사용해준다는 점이다. ###
        node = need_visit.pop(0)
        
        if node not in visited:
            visited.append(node)
            need_visit.extend(graph[node])

    # while 문 종료 조건 : queue가 비게 될 경우 == 더 이상 방문해야할 게 없을때
    return visited


result = bfs(graph=graph, start_node="A")

print(result)