#include <matcher/Matcher.h>

Matcher::Matcher(boost::filesystem::path& inputFile, std::ofstream* outputStream, tools::Options& options)
: GRID_DIM_X(options.dimX), GRID_DIM_Y(options.dimY), WEIGHTS_OFFSET(10*options.threshold), THRESHOLD(options.threshold * options.threshold), PARTICLE_COUNT(options.particleCount)
{
    this->outStream = outputStream;
    this->positionsReader     = new deserializers::PositionsReader(inputFile, options);
    this->previousCoordinates = new std::vector<geometry::Point2D>(); previousCoordinates->reserve(MATCHER_BUFFER_SZ);
    this->currentCoordinates  = new std::vector<geometry::Point2D>(); currentCoordinates->reserve( MATCHER_BUFFER_SZ);
    this->previousTracks      = new std::vector<uint64_t>(); previousTracks->reserve(MATCHER_BUFFER_SZ);
    this->currentTracks       = new std::vector<uint64_t>(); currentTracks->reserve( MATCHER_BUFFER_SZ);
}
Matcher::~Matcher(){
    delete this->positionsReader;
    delete this->previousCoordinates;
    delete this->currentCoordinates;
    delete this->previousTracks;
    delete this->currentTracks;
}

void Matcher::match(){
    BOOST_LOG_TRIVIAL(info) << "Matching data.";
    igraph_t graph;
    igraph_vector_t edges;
    igraph_vector_bool_t types;
    igraph_vector_t weights;
    igraph_vector_long_t matching;
    igraph_integer_t matching_size;
    igraph_real_t matching_weight;

    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);
    igraph_vector_bool_init(&types, 0);
    igraph_vector_init(&edges, 0);
    igraph_vector_init(&weights, 0);
    igraph_vector_long_init(&matching, 0);

    igraph_vector_reserve(&edges, PARTICLE_COUNT);
    igraph_vector_bool_reserve(&types, MATCHER_BUFFER_SZ + MATCHER_BUFFER_SZ);
    igraph_vector_reserve(&weights, MATCHER_BUFFER_SZ * MATCHER_BUFFER_SZ);

    uint64_t trackCounter = 0;

    // Bootstrapping
    this->positionsReader->nextFrame(this->currentCoordinates);
    for(size_t i = 0; i < currentCoordinates->size(); ++i) {
        currentTracks->push_back(trackCounter++);
    }
    // frame-header
    uint32_t size = this->currentCoordinates->size();
    this->outStream->write((char*)&size, sizeof(uint32_t));
    // frame-body
    for(size_t c = 0; c < this->currentCoordinates->size(); ++c){
        const uint64_t TRACK_ID = (*currentTracks)[c];
        const double X = (*currentCoordinates)[c].x();
        const double Y = (*currentCoordinates)[c].y();
        this->outStream->write((char*)&TRACK_ID, sizeof(uint64_t));
        this->outStream->write((char*)&X, sizeof(double));
        this->outStream->write((char*)&Y, sizeof(double));
    }

    size_t t = 0;
    while(this->positionsReader->hasNext()) {
        // Updating
        {
            std::vector<geometry::Point2D>* tmp = previousCoordinates;
            previousCoordinates = currentCoordinates;
            currentCoordinates = tmp;
        }
        {
            std::vector<uint64_t>* tmp = previousTracks;
            previousTracks = currentTracks;
            currentTracks = tmp;
        }

        // Clearing (to prepare for new data)
        currentCoordinates->resize(0); currentTracks->resize(0);
        igraph_vector_clear(&edges);
        igraph_vector_bool_clear(&types);
        igraph_vector_clear(&weights);
        igraph_delete_vertices(&graph, igraph_vss_all());

        // Deserializing
        positionsReader->nextFrame(this->currentCoordinates);

        // Create graph
        const uint32_t N_P = previousCoordinates->size();
        const uint32_t N_C = currentCoordinates->size();
        igraph_add_vertices(&graph, N_P + N_C, 0);
        igraph_vector_bool_resize(&types, N_P + N_C);
        igraph_vector_bool_fill(&types, false);
        igraph_vector_resize(&weights, N_P * N_C);
        currentTracks->resize(N_C);

        // Create the graph
        size_t n = 0;
        for(size_t p = 0; p < N_P; ++p) {
            for(size_t c = 0; c < N_C; ++c) {
                VECTOR(types)[c + N_P] = true;
                const double dX = (*currentCoordinates)[c].x() - (*previousCoordinates)[p].x();
                const double dY = (*currentCoordinates)[c].y() - (*previousCoordinates)[p].y();

                double dX_NPI = 0;
                double dY_NPI = 0;
                const double dXabs = fabs(dX);
                const double dYabs = fabs(dY);

                if(dXabs > GRID_DIM_X / 2.0) {
                    const double d = GRID_DIM_X - dXabs;
                    dX_NPI = (dX > 0) ? - d : + d;
                }
                else {
                    dX_NPI = dX;
                }
                if (dYabs > GRID_DIM_Y / 2.0) {
                    const double d = GRID_DIM_Y - dYabs;
                    dY_NPI = (dY > 0) ? - d : + d;
                }
                else {
                    dY_NPI = dY;
                }

                const double squaredDistance = dX_NPI * dX_NPI + dY_NPI * dY_NPI;
                if(squaredDistance < THRESHOLD) {
                    igraph_vector_push_back(&edges, p);
                    igraph_vector_push_back(&edges, c + N_P);
                    VECTOR(weights)[n] = WEIGHTS_OFFSET - std::sqrt(squaredDistance); // mirror the value and offset into positive intercept
                    ++n;
                }
            }
        }
        igraph_add_edges(&graph, &edges, 0);
        // Find the weighted bipartite matching of maximal cardinality
        igraph_maximum_bipartite_matching(&graph, &types, &matching_size, &matching_weight, &matching, &weights, std::numeric_limits<double>::epsilon());
        t++;
        // Transfer identifications while taking creation and annihilation into account
        for(size_t i = 0; i < igraph_vector_long_size(&matching); ++i){
            // getting matched node id
            const igraph_integer_t M = VECTOR(matching)[i];

            // previous node, i in [0,N_P[
            if(i < N_P) {
                // has been annihilated
                if (M == -1) {
                    trackCounter++;
                }
                // matched to current node
                else {
                    // offset matched node which is a current node id to base zero
                    (*currentTracks)[M - N_P] = (*previousTracks)[i];
                }
            }
            // current node, i in [N_P,N_P+N_C[
            else {
               // has been created
               if (M == -1) {
                   // offset current node id to base zero
                   // add new track id to tracklist and increase counter
                   (*currentTracks)[i - N_P] = trackCounter++;
               }
               // matches a previous node
               else {
                   // already processed
               }
            }
        }

        // frame-header
        this->outStream->write((char*)&N_C, sizeof(uint32_t));

        // frame-body
        for(size_t c = 0; c < N_C; ++c){

            const uint64_t TRACK_ID = (*currentTracks)[c];
            const double X = (*currentCoordinates)[c].x();
            const double Y = (*currentCoordinates)[c].y();
            this->outStream->write((char*)&TRACK_ID, sizeof(uint64_t));
            this->outStream->write((char*)&X, sizeof(double));
            this->outStream->write((char*)&Y, sizeof(double));
        }
    }

    this->outStream->close();

    igraph_vector_destroy(&edges);
    igraph_vector_bool_destroy(&types);
    igraph_vector_long_destroy(&matching);
    igraph_vector_destroy(&weights);

    igraph_destroy(&graph);
}
