class Node:
  def __init__(self) -> None:
    self.children = dict()

inFile = open('book.txt', 'r')
outFile = open('book', 'w')
root = Node()

def addLine(n, moves):
  for m in moves:
    if not (m in n.children):
      n.children[m] = Node()
    n = n.children[m]

def dfs(n):
  for k in n.children.keys():
    print(k, end=' ', file=outFile)
    dfs(n.children[k])
    print('~ ', end=' ', file=outFile)

for l in inFile:
  addLine(root, l.split())

dfs(root)

