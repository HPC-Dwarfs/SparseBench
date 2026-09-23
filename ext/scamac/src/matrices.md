\page MatricesPage List of matrices &amp; benchmark suites
\tableofcontents
\warning Most matrix examples are still missing. Benchmark suites are currently included only for testing. Documentation remains rudimentary.

The complete list of \ref Matrices and \ref Benchmarks of ScaMaC.

ScaMaC provides 15 matrices (with 95 parameters in total) and 1 benchmark suites.


\section Parameters Parameter format

Matrix parameters are listed in the respective
<b>Parameters</b> section
of each matrix, in the following tabular form (the header is only included here for explanation).

Type | Name | Description | Default
-----|------|-------------|--------
int  | <span class="paramname">N</span>  |number of row|  <b>10</b>
Option | <span class="paramname">shuffle</span> | Diagonality | {yes, <b>no</b>}

In this example, the matrix has two parameters:
(i) the integer-valued <span class="paramname">N</span>, with default value <span class="paramname">N</span>=10, and (ii) an option <span class="paramname">shuffle</span> that can be either "yes" or "no", with default value <span class="paramname">shuffle</span>=no.

In addition to the standard types <em>int</em> and <em>double</em>,
the following parameter types are used:

\anchor Seed
\ref Seed "Seed"
random seed -- see scamac_generator_set_rngseed() and scamac_generator_set_rngseed_str() for further information

Parameters can be subject to additional constraints, which are listed in the optional <b>Constraints</b> section of each matrix.

Type | Constraint | Description
-----|------------|------------
[Error] | <span class="paramname">N</span> > 0 | <span class="paramname">N</span> should be positive
[Warning] | <span class="paramname">shuffle</span> != yes | Shuffling is not recommended


\section Matrices Matrices

\subsection Alphabetical_list Alphabetical list
\ref Anderson "Anderson" &mdash; \ref DiagonalReal "DiagonalReal" &mdash; \ref Exciton "Exciton" &mdash; \ref FreeBosonChain "FreeBosonChain" &mdash; \ref FreeFermionChain "FreeFermionChain" &mdash; \ref Harmonic "Harmonic" &mdash; \ref Hubbard "Hubbard" &mdash; \ref Membrane1 "Membrane1" &mdash; \ref OneFermion "OneFermion" &mdash; \ref SpinChainXXZ "SpinChainXXZ" &mdash; \ref TightBinding "TightBinding" &mdash; \ref TopIns "TopIns" &mdash; \ref Tridiagonal "Tridiagonal" &mdash; \ref TridiagonalComplex "TridiagonalComplex" &mdash; \ref TridiagonalReal "TridiagonalReal"

\subsection List_by_groups List by groups
\par (::)
  \ref Anderson "Anderson"  &mdash; \ref DiagonalReal "DiagonalReal"  &mdash; \ref Exciton "Exciton"  &mdash; \ref FreeBosonChain "FreeBosonChain"  &mdash; \ref FreeFermionChain "FreeFermionChain"  &mdash; \ref Harmonic "Harmonic"  &mdash; \ref Hubbard "Hubbard"  &mdash; \ref Membrane1 "Membrane1"  &mdash; \ref OneFermion "OneFermion"  &mdash; \ref SpinChainXXZ "SpinChainXXZ"  &mdash; \ref TightBinding "TightBinding"  &mdash; \ref TopIns "TopIns"  &mdash; \ref Tridiagonal "Tridiagonal"  &mdash; \ref TridiagonalComplex "TridiagonalComplex"  &mdash; \ref TridiagonalReal "TridiagonalReal" 

\subsection Aliases Aliases


<hr>
\subsection Anderson Anderson 
Anderson model of localization in 1D, 2D, 3D

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Lx</td><td>dimensions of cuboid along x-axis</td><td><b>5</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Ly</td><td>dimensions of cuboid along y-axis</td><td><b>5</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Lz</td><td>dimensions of cuboid along z-axis</td><td><b>5</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">t</td><td>hopping strength</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double &ge; 0</td><td class="paramname">ranpot</td><td>random on-site potential [-ranpot, ranpot]</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">boundary_conditions</td><td>open or periodic boundary conditions</td><td>{<b>open</b>, periodic}</td></tr>
<tr><td class="paramtype">\ref Seed</td><td class="paramname">seed</td><td>random seed</td><td><b>1</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">sweep</td><td>mode of traversal of cuboid</td><td>{<b>simple</b>, backforth}</td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">Lx</span>  > 0</td><td>[Parameter range] Lx  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">Ly</span>  > 0</td><td>[Parameter range] Ly  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">Lz</span>  > 0</td><td>[Parameter range] Lz  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">ranpot</span>  >= 0</td><td>[Parameter range] ranpot  >= 0</td></tr>
</table>

\par Description
The Anderson model describes the motion of a quantum-mechanical particle in a disordered solid,
which is represented as a cuboid of <span class="paramname"><b>Lx</b></span>*<span class="paramname"><b>Ly</b></span>*<span class="paramname"><b>Lz</b></span> lattice sites (this is also the matrix dimension).
The kinetic energy is given in the tight-binding approximation (parameter <span class="paramname"><b>t</b></span>), the disorder is given by a random on-site potential (parameter <span class="paramname"><b>ranpot</b></span>).
A 1D or 2D solid is obtained if, e.g., <span class="paramname"><b>Ly</b></span>=<span class="paramname"><b>Lz</b></span> =1 (for 1D) or <span class="paramname"><b>Lz</b></span> =1 (for 2D).

\par Matrix dimension
Matrix dimension is <span class="paramname"><b>Lx</b></span>*<span class="paramname"><b>Ly</b></span>*<span class="paramname"><b>Lz</b></span>

\par Benchmark suites
 \ref Simple 
\subsection DiagonalReal DiagonalReal 
Diagonal (test) matrix

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n</td><td>matrix dimension</td><td><b>100</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">dmin</td><td>minimal diagonal element</td><td><b>-1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">dmax</td><td>maximal diagonal element</td><td><b>1.0</b></td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Error]</td><td><span class="paramname">dmin</span> < <span class="paramname">dmax</span></td><td></td></tr>
<tr><td>[Input]</td><td><span class="paramname">n</span>  > 0</td><td>[Parameter range] n  > 0</td></tr>
</table>

\par Description
A simple diagonal (test) matrix, with diagonal elements distributed uniformly in the interval [<span class="paramname"><b>dmin</b></span>, <span class="paramname"><b>dmax</b></span>] (including the end points).

\par Matrix dimension
dimension = <span class="paramname"><b>n</b></span>
\subsection Exciton Exciton 
Exciton on a lattice

\par Properties
Complex Hermitian


\par Parameters
<table>
<tr><td class="paramtype">double</td><td class="paramname">so</td><td>spin orbit</td><td><b>128.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">ex</td><td>exchange</td><td><b>666.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">mlh</td><td>mass light hole</td><td><b>0.16</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">mhh</td><td>mass heavy hole</td><td><b>3.1</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">me</td><td>mass electron</td><td><b>0.99</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">eps</td><td>dielectric constant</td><td><b>6.94</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">lc</td><td>eff. Coulomb length</td><td><b>1.75</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">kx</td><td>momentum kx</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">ky</td><td>momentum ky</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">kz</td><td>momentum kz</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">a</td><td>lattice constant</td><td><b>0.42696</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">L</td><td>cube length</td><td><b>10</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">symm</td><td>symmetry</td><td>{<b>para</b>, ortho}</td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">L</span>  > 0</td><td>[Parameter range] L  > 0</td></tr>
</table>

\par Description
This matrix describes an exciton, the bound electron+hole state in a semiconductor, within a microscopic lattice model. The default parameter values correspond to the cuprous oxide Cu2O, as investigated in Ref. [...].
\subsection FreeBosonChain FreeBosonChain 
Free bosons on a chain

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">double</td><td class="paramname">t</td><td>hopping strength</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_species</td><td>number of bosonic species</td><td><b>1</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_sites</td><td>number of sites</td><td><b>10</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_bosons</td><td>number of bosons per species</td><td><b>5</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">bc</td><td>open (false) or periodic (true) boundary conditions</td><td>{open, <b>periodic</b>}</td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">n_species</span>  > 0</td><td>[Parameter range] n_species  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_sites</span>  > 0</td><td>[Parameter range] n_sites  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_bosons</span>  > 0</td><td>[Parameter range] n_bosons  > 0</td></tr>
</table>

\par Description
This matrix is a simple example for a non-interacting many-particle quantum system, where bosons move on a chain with <span class="paramname"><b>n\_sites</b></span> sites and nearest-neighbour hopping (parameter <span class="paramname"><b>t</b></span>). The bosons form <span class="paramname"><b>n\_species</b></span> different species, with <span class="paramname"><b>n\_bosons</b></span> each.

\par Matrix dimension
The matrix dimension grows roughly exponentially as a function of <span class="paramname"><b>n\_bosons</b></span> and <span class="paramname"><b>n\_sites</b></span>.
\subsection FreeFermionChain FreeFermionChain 
Free fermions on a chain

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">double</td><td class="paramname">t</td><td>hopping strength</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_species</td><td>number of fermionic species</td><td><b>1</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_sites</td><td>number of sites</td><td><b>10</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_fermions</td><td>number of fermions per species</td><td><b>5</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">bc</td><td>open (false) or periodic (true) boundary conditions</td><td>{open, <b>periodic</b>}</td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Error]</td><td><span class="paramname">n_fermions</span> <= <span class="paramname">n_sites</span></td><td></td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_species</span>  > 0</td><td>[Parameter range] n_species  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_sites</span>  > 0</td><td>[Parameter range] n_sites  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_fermions</span>  > 0</td><td>[Parameter range] n_fermions  > 0</td></tr>
</table>

\par Description
This matrix is a simple example for a non-interacting many-particle quantum system, where fermions move on a chain with <span class="paramname"><b>n\_sites</b></span> sites and nearest-neighbour hopping (parameter <span class="paramname"><b>t</b></span>). The fermions form <span class="paramname"><b>n\_species</b></span> different species, with <span class="paramname"><b>n\_fermions</b></span> each. As a consequence of the Pauli exclusion principle for fermions, there cannot be more fermions than sites.

\par Matrix dimension
The matrix dimension grows roughly exponentially as a function of <span class="paramname"><b>n\_fermions</b></span> and <span class="paramname"><b>n\_sites</b></span>.
\subsection Harmonic Harmonic 
Quantum harmonic oscillator

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">double</td><td class="paramname">omega</td><td>oscillator frequency</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">lambda</td><td>oscillator shift</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_bos</td><td>number of bosons</td><td><b>100</b></td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">n_bos</span>  > 0</td><td>[Parameter range] n_bos  > 0</td></tr>
</table>

\par Description
The quantum harmonic oscillator in occupation number representation.
The resulting matrix is tridiagonal.

\par Matrix dimension
dimension = <span class="paramname"><b>n\_bos</b></span>
\subsection Hubbard Hubbard 
The 1D Hubbard model from quantum physics

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">double</td><td class="paramname">t</td><td>hopping strength</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">U</td><td>Hubbard interaction</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_sites</td><td>number of sites</td><td><b>10</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_fermions</td><td>number of fermions per spin orientation</td><td><b>5</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">boundary_conditions</td><td>open or periodic boundary conditions</td><td>{<b>open</b>, periodic}</td></tr>
<tr><td class="paramtype">double</td><td class="paramname">ranpot</td><td>random on-site potential [-ranpot, ranpot]</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">\ref Seed</td><td class="paramname">seed</td><td>random seed</td><td><b>1</b></td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Error]</td><td><span class="paramname">n_fermions</span> <= <span class="paramname">n_sites</span></td><td></td></tr>
<tr><td>[Warning]</td><td><span class="paramname">n_sites</span> <= 1000</td><td>very large n_sites (>1000)</td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_sites</span>  > 0</td><td>[Parameter range] n_sites  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_fermions</span>  > 0</td><td>[Parameter range] n_fermions  > 0</td></tr>
</table>

\par Description
The Hubbard model is a prime example of a strongly correlated interacting many-particle quantum system. This matrix implements the 1D Hubbard model, where <span class="paramname"><b>n\_fermions</b></span> electrons with "spin up" or "spin down" move on a chain with n\_sites.
The kinetic energy is controlled by the parameter <span class="paramname"><b>t</b></span>, the Coulomb interaction (repulsive for <span class="paramname"><b>U</b></span>>0, attractive for <span class="paramname"><b>U</b></span><0) by the parameter <span class="paramname"><b>U</b></span>.
For <span class="paramname"><b>U</b></span>=0, one obtains a special case of the FreeFermionChain.

\par Matrix dimension
The matrix dimension is \\binomial(<span class="paramname"><b>n\_sites</b></span>,<span class="paramname"><b>n\_up</b></span>)^2.

\par Benchmark suites
 \ref Simple 
\subsection Membrane1 Membrane1 
Damped vibrations of a thin rectangular membrane (friction type 1)

\par Properties
Real General


\par Parameters
<table>
<tr><td class="paramtype">double &gt; 0</td><td class="paramname">lx</td><td>length side x</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double &gt; 0</td><td class="paramname">ly</td><td>length side y</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">rho</td><td>mass density</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">cwave</td><td>wave velocity</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">sigma</td><td>friction strength</td><td><b>0.1</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">nx</td><td>discretization points along side x</td><td><b>10</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">ny</td><td>discretization points along side y</td><td><b>10</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">pack</td><td>choose small (=tight) or large (=loose) 2x2 blocks</td><td>{<b>tight</b>, loose}</td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">lx</span>  > 0</td><td>[Parameter range] lx  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">ly</span>  > 0</td><td>[Parameter range] ly  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">nx</span>  > 0</td><td>[Parameter range] nx  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">ny</span>  > 0</td><td>[Parameter range] ny  > 0</td></tr>
</table>
\subsection OneFermion OneFermion 
one fermion on a chain

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">double</td><td class="paramname">t</td><td>hopping strength</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">int</td><td class="paramname">n_sites</td><td>number of sites</td><td><b>10</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">bc</td><td>boundary conditions</td><td>{open, <b>periodic</b>}</td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Error]</td><td><span class="paramname">n_sites</span> > 0</td><td></td></tr>
<tr><td>[Warning]</td><td><span class="paramname">n_sites</span> <= 1000</td><td>n_sites is very large</td></tr>
</table>
\subsection SpinChainXXZ SpinChainXXZ 
One-dimensional XXZ model

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">double</td><td class="paramname">Jxy</td><td>J_x=J_y</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">Jz</td><td>J_z</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">Bz</td><td>B_z</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_sites</td><td>number of sites</td><td><b>10</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">n_up</td><td>number of _up_ spins</td><td><b>5</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">boundary_conditions</td><td>boundary conditions</td><td>{<b>open</b>, periodic}</td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Error]</td><td><span class="paramname">n_up</span> <= <span class="paramname">n_sites</span></td><td></td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_sites</span>  > 0</td><td>[Parameter range] n_sites  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">n_up</span>  > 0</td><td>[Parameter range] n_up  > 0</td></tr>
</table>

\par Description
Spin chains are examples of strongly correlated interacting many-particle quantum systems. This example implements the XXZ-model, where <span class="paramname"><b>n\_sites</b></span> spin are arranged as a 1D chain. Neighboring spins interact via magnetic (i.e., Heisenberg) exchange. The interaction strenght is controlled by parameters <span class="paramname"><b>Jxy</b></span> (in the xy-plane) and <span class="paramname"><b>Jz</b></span> (in the z-direction, perpendicular to the plane).
For <span class="paramname"><b>Jxy</b></span>=<span class="paramname"><b>Jz</b></span>, the interaction is isotropic.
Also included is a magnetic field in the z-direction (parameter <span class="paramname"><b>Bz</b></span>).
The number of "up" spins is fixed by the parameter <span class="paramname"><b>n\_up</b></span>.

\par Matrix dimension
The matrix dimension is \\binomial(<span class="paramname"><b>n\_sites</b></span>, <span class="paramname"><b>n\_up</b></span>).
\subsection TightBinding TightBinding 
simple 1D (chain), 2D (square), or 3D (cubic) tight-binding model

\par Properties
Real Symmetric


\par Parameters
<table>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Lx</td><td>dimensions of cuboid (x-axis)</td><td><b>5</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Ly</td><td>dimensions of cuboid (y-axis)</td><td><b>5</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Lz</td><td>dimensions of cuboid (z-axis)</td><td><b>5</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">t</td><td>hopping strength</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">boundary_conditions</td><td>boundary conditions</td><td>{<b>open</b>, periodic}</td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">mx</td><td>subdivision of cuboid (x-axis)</td><td><b>1</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">my</td><td>subdivision of cuboid (y-axis)</td><td><b>1</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">mz</td><td>subdivision of cuboid (z-axis)</td><td><b>1</b></td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">Lx</span>  > 0</td><td>[Parameter range] Lx  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">Ly</span>  > 0</td><td>[Parameter range] Ly  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">Lz</span>  > 0</td><td>[Parameter range] Lz  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">mx</span>  > 0</td><td>[Parameter range] mx  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">my</span>  > 0</td><td>[Parameter range] my  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">mz</span>  > 0</td><td>[Parameter range] mz  > 0</td></tr>
</table>
\subsection TopIns TopIns 
3D topological insulator

\par Properties
Complex Hermitian


\par Parameters
<table>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Lx</td><td>dimensions of cuboid (x-axis)</td><td><b>5</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Ly</td><td>dimensions of cuboid (y-axis)</td><td><b>5</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">Lz</td><td>dimensions of cuboid (z-axis)</td><td><b>5</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">t</td><td>hopping strength</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">m</td><td>tuning parameter</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">D1</td><td>symmetry breaking (IS+TRS)</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">D2</td><td>symmetry breaking (IS)</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">ranpot</td><td>random on-site potential [-ranpot, ranpot]</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">Option</td><td class="paramname">boundary_conditions</td><td>boundary conditions</td><td>{<b>open</b>, periodic}</td></tr>
<tr><td class="paramtype">\ref Seed</td><td class="paramname">seed</td><td>random seed</td><td><b>1</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">mx</td><td>subdivision of cuboid (x-axis)</td><td><b>1</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">my</td><td>subdivision of cuboid (y-axis)</td><td><b>1</b></td></tr>
<tr><td class="paramtype">int &gt; 0</td><td class="paramname">mz</td><td>subdivision of cuboid (z-axis)</td><td><b>1</b></td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">Lx</span>  > 0</td><td>[Parameter range] Lx  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">Ly</span>  > 0</td><td>[Parameter range] Ly  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">Lz</span>  > 0</td><td>[Parameter range] Lz  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">mx</span>  > 0</td><td>[Parameter range] mx  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">my</span>  > 0</td><td>[Parameter range] my  > 0</td></tr>
<tr><td>[Input]</td><td><span class="paramname">mz</span>  > 0</td><td>[Parameter range] mz  > 0</td></tr>
</table>
\subsection Tridiagonal Tridiagonal 
Tridiagonal matrix

\par Properties
Complex Hermitian


\par Parameters
<table>
<tr><td class="paramtype">idx &gt; 0</td><td class="paramname">n</td><td>matrix dimension</td><td><b>100</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">diag</td><td>diagonal element</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">offdiag</td><td>off-diagonal element</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">phi</td><td>phase for off-diagonal element</td><td><b>0.0</b></td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">n</span>  > 0</td><td>[Parameter range] n  > 0</td></tr>
</table>
\subsection TridiagonalComplex TridiagonalComplex 
Non-symmetric tridiagonal matrix

\par Properties
Complex General


\par Parameters
<table>
<tr><td class="paramtype">idx &gt; 0</td><td class="paramname">n</td><td>matrix dimension</td><td><b>100</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">diag_re</td><td>diagonal element</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">diag_im</td><td>diagonal element</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">subdiag</td><td>off-diagonal element (below diagonal)</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">supdiag</td><td>off-diagonal element (above diagonal)</td><td><b>1.0</b></td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">n</span>  > 0</td><td>[Parameter range] n  > 0</td></tr>
</table>
\subsection TridiagonalReal TridiagonalReal 
Non-symmetric tridiagonal matrix

\par Properties
Real General


\par Parameters
<table>
<tr><td class="paramtype">idx &gt; 0</td><td class="paramname">n</td><td>matrix dimension</td><td><b>100</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">diag</td><td>diagonal element</td><td><b>0.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">subdiag</td><td>off-diagonal element (below diagonal)</td><td><b>1.0</b></td></tr>
<tr><td class="paramtype">double</td><td class="paramname">supdiag</td><td>off-diagonal element (above diagonal)</td><td><b>1.0</b></td></tr>
</table>
</dd>
</dl>

\par Constraints
<table>
<tr><td>[Input]</td><td><span class="paramname">n</span>  > 0</td><td>[Parameter range] n  > 0</td></tr>
</table>

\section Benchmarks Benchmark suites

 \ref Simple 
\subsection Simple
Simple suite

\par Description
Presently, in ScaMaC v0.8.2, the benchmark suite is included for testing purposes only.

\par List of matrices (BmSimple-m-x)

<b>BmSimple-1-x</b> = \ref Hubbard "Hubbard"  

\par
The Hubbard model is included because of its ubiquity.

<b>BmSimple-2-x</b> = \ref Anderson "Anderson"  

