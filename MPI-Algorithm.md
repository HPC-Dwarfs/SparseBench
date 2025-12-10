# Overall MPI algorithm

## Exchange

### Idea

Reduce communication to one exchange routine. All other steps are mostly local
without any MPI call. This is achieved by appending the column indices that are
not available on the current rank to the end of the local vector and rewrite the
column indices in the matrix accordingly.

## Communication data structure

- `totalSendCount`: Total number of elements to send to all receivers
- `elementsToSend[totalSendCount]`: Local element ids to send to all receivers
- `indegree`: Number of ranks we receive messages from
- `outdegree`: Number of ranks we send messages to
- `sources[indegree]`: List of ranks we receive messages from
- `recvCounts[indegree]`: Message counts for messages we receive from senders
- `rdispls[indegree]`: Displacements in receive buffer
- `destinations[outdegree]`: List of ranks we send messages to
- `sendCounts[outdegree]`: Message counts for messages we send to receivers
- `sdispls[outdegree]`: Displacements in send buffer

## Partitioning

The matrix has to be distributed. Every rank gets a consecutive number of rows.
Other options (not implemented):

- Take into account total non zeroes per rank
- Take into account total communication data volume

## Localization

### Step 1: Identify externals and create external lookup

Scan the matrix and find unique external column references:

- Determine if an index targets a local or external value
- If external, check if it was already taken care for:
  - If yes, do nothing
  - otherwise
    - add it to the external lookup (a binary search tree)
    - add it to the external local to global id map
    - increment the external counter

Variables:

- `extCount`: Number of externals for current rank
- `extLookup` binary search tree: Lookup if global index was
  already seen before.
- `extLocalToGlobal` SIZE `externalCount`: List of externals as
  encountered in the matrix mapping from a local id to the global id

### Step 2: Build dist Graph topology providing known incoming edges

- Gather the start row offsets for all ranks
- Determine the owning ranks for all externals
- Set known graph with all incoming edges (all rank we need data from). Edge
  weights are used to communicate message counts.
- Create MPI Dist Graph Topology
- Retrieve complete communication topology for current rank

Variables:

- `extOwningRank` SIZE `extCount`: Owning rank for all externals
- `sourceCount`: How many ranks send us messages
- `recvFromNeighbors` SIZE `size`: Marker from which ranks the current rank
  requires externals and how many

### Step 3 Reorder externals and localize matrix

- Reorder externals to ensure that external belonging to the same rank are
  consecutive
- Rewrite all matrix entries to reference local + external RHS vector

Variables:

- `extLocalToGlobalReordered` SIZE `extCount`: List of externals reordered
  mapping from local external id to global id
- `extLocalIndex` SIZE `extCount`: List of externals mapping from local external
  id to local RHS vector id

### Step 4 Build global index list for external communication

Build a index list to local RHS ids for all elements the current rank needs to
send to other ranks, in the order they require them in their external part of
the RHS vector.

- Send `extLocalToGlobalReordered` to all source ranks we receive messages from
- On the sender side remap the global column ids to local RHS ids
