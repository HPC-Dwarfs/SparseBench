if (!(strcmp(matname,"Anderson"))) {
my_string = malloc(401* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "int Lx [dimensions of cuboid along x-axis]\n",43);
strncpy(&my_string[43], "int Ly [dimensions of cuboid along y-axis]\n",43);
strncpy(&my_string[86], "int Lz [dimensions of cuboid along z-axis]\n",43);
strncpy(&my_string[129], "double t [hopping strength]\n",28);
strncpy(&my_string[157], "double ranpot [random on-site potential [-ranpot, ranpot]]\n",59);
strncpy(&my_string[216], "option boundary_conditions {open,periodic} [open or periodic boundary conditions]\n",82);
strncpy(&my_string[298], "Seed seed [random seed]\n",24);
strncpy(&my_string[322], "option sweep {simple,backforth} [mode of traversal of cuboid]\n",62);
my_string[383]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"DiagonalReal"))) {
my_string = malloc(110* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "int n [matrix dimension]\n",25);
strncpy(&my_string[25], "double dmin [minimal diagonal element]\n",39);
strncpy(&my_string[64], "double dmax [maximal diagonal element]\n",39);
my_string[102]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"Exciton"))) {
my_string = malloc(376* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "double so [spin orbit]\n",23);
strncpy(&my_string[23], "double ex [exchange]\n",21);
strncpy(&my_string[44], "double mlh [mass light hole]\n",29);
strncpy(&my_string[73], "double mhh [mass heavy hole]\n",29);
strncpy(&my_string[102], "double me [mass electron]\n",26);
strncpy(&my_string[128], "double eps [dielectric constant]\n",33);
strncpy(&my_string[161], "double lc [eff. Coulomb length]\n",32);
strncpy(&my_string[193], "double kx [momentum kx]\n",24);
strncpy(&my_string[217], "double ky [momentum ky]\n",24);
strncpy(&my_string[241], "double kz [momentum kz]\n",24);
strncpy(&my_string[265], "double a [lattice constant]\n",28);
strncpy(&my_string[293], "int L [cube length]\n",20);
strncpy(&my_string[313], "option symm {para,ortho} [symmetry]\n",36);
my_string[348]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"FreeBosonChain"))) {
my_string = malloc(235* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "double t [hopping strength]\n",28);
strncpy(&my_string[28], "int n_species [number of bosonic species]\n",42);
strncpy(&my_string[70], "int n_sites [number of sites]\n",30);
strncpy(&my_string[100], "int n_bosons [number of bosons per species]\n",44);
strncpy(&my_string[144], "option bc {open,periodic} [open (false) or periodic (true) boundary conditions]\n",80);
my_string[223]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"FreeFermionChain"))) {
my_string = malloc(241* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "double t [hopping strength]\n",28);
strncpy(&my_string[28], "int n_species [number of fermionic species]\n",44);
strncpy(&my_string[72], "int n_sites [number of sites]\n",30);
strncpy(&my_string[102], "int n_fermions [number of fermions per species]\n",48);
strncpy(&my_string[150], "option bc {open,periodic} [open (false) or periodic (true) boundary conditions]\n",80);
my_string[229]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"Harmonic"))) {
my_string = malloc(105* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "double omega [oscillator frequency]\n",36);
strncpy(&my_string[36], "double lambda [oscillator shift]\n",33);
strncpy(&my_string[69], "int n_bos [number of bosons]\n",29);
my_string[97]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"Hubbard"))) {
my_string = malloc(326* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "double t [hopping strength]\n",28);
strncpy(&my_string[28], "double U [Hubbard interaction]\n",31);
strncpy(&my_string[59], "int n_sites [number of sites]\n",30);
strncpy(&my_string[89], "int n_fermions [number of fermions per spin orientation]\n",57);
strncpy(&my_string[146], "option boundary_conditions {open,periodic} [open or periodic boundary conditions]\n",82);
strncpy(&my_string[228], "double ranpot [random on-site potential [-ranpot, ranpot]]\n",59);
strncpy(&my_string[287], "Seed seed [random seed]\n",24);
my_string[310]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"Membrane1"))) {
my_string = malloc(324* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "double lx [length side x]\n",26);
strncpy(&my_string[26], "double ly [length side y]\n",26);
strncpy(&my_string[52], "double rho [mass density]\n",26);
strncpy(&my_string[78], "double cwave [wave velocity]\n",29);
strncpy(&my_string[107], "double sigma [friction strength]\n",33);
strncpy(&my_string[140], "int nx [discretization points along side x]\n",44);
strncpy(&my_string[184], "int ny [discretization points along side y]\n",44);
strncpy(&my_string[228], "option pack {tight,loose} [choose small (=tight) or large (=loose) 2x2 blocks]\n",79);
my_string[306]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"OneFermion"))) {
my_string = malloc(113* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "double t [hopping strength]\n",28);
strncpy(&my_string[28], "int n_sites [number of sites]\n",30);
strncpy(&my_string[58], "option bc {open,periodic} [boundary conditions]\n",48);
my_string[105]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"SpinChainXXZ"))) {
my_string = malloc(193* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "double Jxy [J_x=J_y]\n",21);
strncpy(&my_string[21], "double Jz [J_z]\n",16);
strncpy(&my_string[37], "double Bz [B_z]\n",16);
strncpy(&my_string[53], "int n_sites [number of sites]\n",30);
strncpy(&my_string[83], "int n_up [number of _up_ spins]\n",32);
strncpy(&my_string[115], "option boundary_conditions {open,periodic} [boundary conditions]\n",65);
my_string[179]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"TightBinding"))) {
my_string = malloc(347* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "int Lx [dimensions of cuboid (x-axis)]\n",39);
strncpy(&my_string[39], "int Ly [dimensions of cuboid (y-axis)]\n",39);
strncpy(&my_string[78], "int Lz [dimensions of cuboid (z-axis)]\n",39);
strncpy(&my_string[117], "double t [hopping strength]\n",28);
strncpy(&my_string[145], "option boundary_conditions {open,periodic} [boundary conditions]\n",65);
strncpy(&my_string[210], "int mx [subdivision of cuboid (x-axis)]\n",40);
strncpy(&my_string[250], "int my [subdivision of cuboid (y-axis)]\n",40);
strncpy(&my_string[290], "int mz [subdivision of cuboid (z-axis)]\n",40);
my_string[329]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"TopIns"))) {
my_string = malloc(542* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "int Lx [dimensions of cuboid (x-axis)]\n",39);
strncpy(&my_string[39], "int Ly [dimensions of cuboid (y-axis)]\n",39);
strncpy(&my_string[78], "int Lz [dimensions of cuboid (z-axis)]\n",39);
strncpy(&my_string[117], "double t [hopping strength]\n",28);
strncpy(&my_string[145], "double m [tuning parameter]\n",28);
strncpy(&my_string[173], "double D1 [symmetry breaking (IS+TRS)]\n",39);
strncpy(&my_string[212], "double D2 [symmetry breaking (IS)]\n",35);
strncpy(&my_string[247], "double ranpot [random on-site potential [-ranpot, ranpot]]\n",59);
strncpy(&my_string[306], "option boundary_conditions {open,periodic} [boundary conditions]\n",65);
strncpy(&my_string[371], "Seed seed [random seed]\n",24);
strncpy(&my_string[395], "int mx [subdivision of cuboid (x-axis)]\n",40);
strncpy(&my_string[435], "int my [subdivision of cuboid (y-axis)]\n",40);
strncpy(&my_string[475], "int mz [subdivision of cuboid (z-axis)]\n",40);
my_string[514]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"Tridiagonal"))) {
my_string = malloc(147* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "idx n [matrix dimension]\n",25);
strncpy(&my_string[25], "double diag [diagonal element]\n",31);
strncpy(&my_string[56], "double offdiag [off-diagonal element]\n",38);
strncpy(&my_string[94], "double phi [phase for off-diagonal element]\n",44);
my_string[137]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"TridiagonalComplex"))) {
my_string = malloc(214* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "idx n [matrix dimension]\n",25);
strncpy(&my_string[25], "double diag_re [diagonal element]\n",34);
strncpy(&my_string[59], "double diag_im [diagonal element]\n",34);
strncpy(&my_string[93], "double subdiag [off-diagonal element (below diagonal)]\n",55);
strncpy(&my_string[148], "double supdiag [off-diagonal element (above diagonal)]\n",55);
my_string[202]=0;
*desc = my_string;
return SCAMAC_EOK;
}

if (!(strcmp(matname,"TridiagonalReal"))) {
my_string = malloc(175* sizeof *my_string);
if (!my_string) {return SCAMAC_EMALLOCFAIL;}
strncpy(&my_string[0], "idx n [matrix dimension]\n",25);
strncpy(&my_string[25], "double diag [diagonal element]\n",31);
strncpy(&my_string[56], "double subdiag [off-diagonal element (below diagonal)]\n",55);
strncpy(&my_string[111], "double supdiag [off-diagonal element (above diagonal)]\n",55);
my_string[165]=0;
*desc = my_string;
return SCAMAC_EOK;
}

