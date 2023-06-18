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


def dfs(graph, start_node):
 
    ## 기본은 항상 두개의 리스트를 별도로 관리해주는 것
    # 1. 방문완료 리스트
    # 2. 방문예정 리스트(need_visit)
    visited, need_visit = list(), list()
 
    ## 시작 노드를 시정하기 
    need_visit.append(start_node)
    
    ## 만약 아직도 방문이 필요한 노드가 있다면,
    while need_visit:
 
        ## 그 중에서 가장 마지막 데이터를 추출 (스택 구조의 활용)
        node = need_visit.pop()
        
        ## 만약 그 노드가 방문한 목록에 없다면
        if node not in visited:
 
            ## 방문한 목록에 추가하기 
            visited.append(node)
 
            ## 그 노드에 연결된 노드를 
            need_visit.extend(graph[node])
            
    return visited


result = dfs(graph, 'A')
print(result)

