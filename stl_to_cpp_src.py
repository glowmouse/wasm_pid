import re

#
# STL to C++ source
#

class LinesWithPutBack:

  def __init__(self, downstream_iter):
    self.downstream_iter = downstream_iter
    self.put_back_line = None

  def __iter__(self):
    return self

  def __next__(self):
    if self.put_back_line:
      return_line = self.put_back_line
      self.put_back_line = None;
      return return_line;
    return self.downstream_iter.__next__();

  def unget( self, put_back_line ):
    self.put_back_line = put_back_line

class Vector:
  def __init__(self, x, y, z, nx, ny, nz ):
    self.x = float(x)
    self.y = float(y)
    self.z = float(z)
    self.nx = float(nx)
    self.ny = float(ny)
    self.nz = float(nz)

  def __eq__(self, other ) :
    return ((self.x, self.y, self.z, self.nx, self.ny, self.nx ) == (other.x, other.y, other.z, other.nx, other.ny, other.nz ))

  def __ne__(self, other ):
    return not ( self == other )

  def __lt__(self, other ):
    return ((self.x, self.y, self.z, self.nx, self.ny, self.nz ) < (other.x, other.y, other.z, other.nx, other.ny, other.nz ))

  def __repr__(self):
    return "<%f %f %f> Normal <%f %f %f>" % (self.x, self.y, self.z, self.nx, self.ny, self.nz )

  def __hash__(self):
    return str(self).__hash__()

class Triangle:
  def __init__(self, v0, v1, v2 ):
    self.v0 = v0
    self.v1 = v1
    self.v2 = v2

class STLModel:

  def __init__(self):
    self.vectors = []
    self.vectorToId = {}
    self.maxId = 0
    self.triangles = []

  def getId( self, vector ):
    if not vector in self.vectorToId:
      index = self.maxId
      self.maxId += 1
      self.vectors.append( vector )
      self.vectorToId[ vector ] = index
    else:
      index = self.vectorToId[ vector ]

    return index

  def parseASCIIStl( self, lineSource ):
    header = next(lineSource);
    assert header.find("solid", 0 ) == 0
    while ( True ):
      triangle = self.parseFacet( lineSource )
      if not triangle:
        break
      self.triangles.append( triangle )   

    footer = next(lineSource);
    assert footer.find("endsolid", 0 ) == 0
    
  def parseFacet( self, lineSource ):
    line = next(lineSource);
    numberRegex = r"([-+]?\d*\.\d+|[-+]?\d+|[-+]?\d+\.\d+e[-+]?\d+)"
    normalRegex = r"^\s*facet\s+normal\s+" + numberRegex + "\s+" + numberRegex + "\s+" + numberRegex + "\s*$"
    normalRe = re.search( normalRegex, line )
    if not normalRe:
      lineSource.unget( line )
      return None
    [nx, ny, nz] = [ normalRe.group(1), normalRe.group(2), normalRe.group(3) ]
    assert re.search( r"^\s+outer loop$", next( lineSource ))
    vertexRegex = r"^\s+vertex\s+" + numberRegex + "\s+" + numberRegex + "\s+" + numberRegex + "\s*$"

    vertexRe = re.search( vertexRegex, next( lineSource ))
    assert vertexRe
    v0 = Vector( vertexRe.group(1), vertexRe.group(2), vertexRe.group(3), nx, ny, nz )
    vertexRe = re.search( vertexRegex, next( lineSource ))
    assert vertexRe
    v1 = Vector( vertexRe.group(1), vertexRe.group(2), vertexRe.group(3), nx, ny, nz )
    vertexRe = re.search( vertexRegex, next( lineSource ))
    assert vertexRe
    v2 = Vector( vertexRe.group(1), vertexRe.group(2), vertexRe.group(3), nx, ny, nz )
  
    assert re.search( r"^\s+endloop$", next( lineSource ))
    assert re.search( r"^\s+endfacet$", next( lineSource ))
    return Triangle( self.getId(v0), self.getId(v1), self.getId(v2) )

  def dump(self):
    print("#define TRIANGLES " + str( len(self.triangles) ) + "\n" )
    print("    MatrixXu indices(3, " + str( len(self.triangles) ) + ");" )
    index = 0;
    for triangle in self.triangles:
      print("    indices.col(%d) << %d,%d,%d;" % (index,triangle.v0,triangle.v1,triangle.v2))
      index+=1

    print("    MatrixXf positions(3, " + str(self.maxId) +");" ) 
    index = 0;
    for vector in self.vectors:
      print("    positions.col(%d) << %f,%f,%f;" % ( index, vector.x, vector.y, vector.z) )
      index+=1

    print("    MatrixXf normals(3, " + str(self.maxId) +");" ) 
    index = 0;
    for vector in self.vectors:
      print("    normals.col(%d) << %f,%f,%f;" % ( index, vector.nx, vector.ny, vector.nz) )
      index+=1

file = open('arm_base.stl', 'r' )
lineSource = LinesWithPutBack( file )
model = STLModel()
model.parseASCIIStl( lineSource )
model.dump()

