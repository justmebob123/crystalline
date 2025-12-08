#include "recovery_common/recovery_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * File I/O Module
 * 
 * Handles loading and saving data in multiple formats:
 * - Geometric: OBJ, STL, PLY, JSON
 * - Signal: WAV, FLAC, MP3, CSV, JSON
 * - Image: JPG, PNG, TIFF, DICOM
 * - Network: GML, GraphML, JSON
 * - Crypto: JSON, binary
 * - Scientific: PDB, MOL2, CIF, JSON
 * - ML: PT, H5, NPY, ONNX, JSON
 */

// Forward declarations for format-specific loaders/savers
static bool load_obj(FILE* file, GeometricData* data);
static bool load_stl(FILE* file, GeometricData* data);
static bool load_ply(FILE* file, GeometricData* data);
static bool load_wav(FILE* file, SignalData* data);
static bool load_csv_signal(FILE* file, SignalData* data);
static bool load_image(FILE* file, ImageData* data, FileFormat format);
static bool load_network(FILE* file, NetworkData* data, FileFormat format);
static bool load_pdb(FILE* file, ScientificData* data);
static bool load_ml(FILE* file, MLData* data, FileFormat format);
static bool load_json(FILE* file, GenericData* data, DataType* type);
static bool save_obj(FILE* file, GeometricData* data);
static bool save_stl(FILE* file, GeometricData* data);
static bool save_ply(FILE* file, GeometricData* data);
static bool save_wav(FILE* file, SignalData* data);
static bool save_csv_signal(FILE* file, SignalData* data);
static bool save_image(FILE* file, ImageData* data, FileFormat format);
static bool save_network(FILE* file, NetworkData* data, FileFormat format);
static bool save_pdb(FILE* file, ScientificData* data);
static bool save_ml(FILE* file, MLData* data, FileFormat format);
static bool save_json(FILE* file, GenericData* data, DataType type);

// ============================================================================
// FORMAT DETECTION
// ============================================================================

FileFormat detect_format(const char* filename) {
    if (!filename) {
        return FORMAT_UNKNOWN;
    }
    
    // Find file extension
    const char* ext = strrchr(filename, '.');
    if (!ext) {
        return FORMAT_UNKNOWN;
    }
    ext++; // Skip the dot
    
    // Convert to lowercase for comparison
    char lower_ext[32];
    size_t i;
    for (i = 0; i < sizeof(lower_ext) - 1 && ext[i]; i++) {
        lower_ext[i] = tolower(ext[i]);
    }
    lower_ext[i] = '\0';
    
    // Geometric formats
    if (strcmp(lower_ext, "obj") == 0) return FORMAT_OBJ;
    if (strcmp(lower_ext, "stl") == 0) return FORMAT_STL;
    if (strcmp(lower_ext, "ply") == 0) return FORMAT_PLY;
    
    // Signal formats
    if (strcmp(lower_ext, "wav") == 0) return FORMAT_WAV;
    if (strcmp(lower_ext, "flac") == 0) return FORMAT_FLAC;
    if (strcmp(lower_ext, "mp3") == 0) return FORMAT_MP3;
    
    // Image formats
    if (strcmp(lower_ext, "jpg") == 0 || strcmp(lower_ext, "jpeg") == 0) return FORMAT_JPG;
    if (strcmp(lower_ext, "png") == 0) return FORMAT_PNG;
    if (strcmp(lower_ext, "tiff") == 0 || strcmp(lower_ext, "tif") == 0) return FORMAT_TIFF;
    if (strcmp(lower_ext, "dcm") == 0 || strcmp(lower_ext, "dicom") == 0) return FORMAT_DICOM;
    
    // Network formats
    if (strcmp(lower_ext, "gml") == 0) return FORMAT_GML;
    if (strcmp(lower_ext, "graphml") == 0) return FORMAT_GRAPHML;
    
    // Scientific formats
    if (strcmp(lower_ext, "pdb") == 0) return FORMAT_PDB;
    if (strcmp(lower_ext, "mol2") == 0) return FORMAT_MOL2;
    if (strcmp(lower_ext, "cif") == 0) return FORMAT_CIF;
    
    // ML formats
    if (strcmp(lower_ext, "pt") == 0 || strcmp(lower_ext, "pth") == 0) return FORMAT_PT;
    if (strcmp(lower_ext, "h5") == 0 || strcmp(lower_ext, "hdf5") == 0) return FORMAT_H5;
    if (strcmp(lower_ext, "npy") == 0) return FORMAT_NPY;
    if (strcmp(lower_ext, "onnx") == 0) return FORMAT_ONNX;
    
    // Common formats
    if (strcmp(lower_ext, "csv") == 0) return FORMAT_CSV;
    if (strcmp(lower_ext, "json") == 0) return FORMAT_JSON;
    
    return FORMAT_UNKNOWN;
}

// ============================================================================
// GENERIC DATA LOADING
// ============================================================================

GenericData* load_data(const char* filename, DataType* type, FileFormat* format) {
    if (!filename || !type || !format) {
        set_error(ERROR_INVALID_PARAMETER, "Invalid parameters to load_data");
        return NULL;
    }
    
    // Detect format
    *format = detect_format(filename);
    if (*format == FORMAT_UNKNOWN) {
        set_error(ERROR_INVALID_FORMAT, "Unknown file format");
        return NULL;
    }
    
    // Open file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        set_error(ERROR_FILE_NOT_FOUND, "Could not open file");
        return NULL;
    }
    
    // Allocate generic data
    GenericData* data = (GenericData*)calloc(1, sizeof(GenericData));
    if (!data) {
        fclose(file);
        set_error(ERROR_OUT_OF_MEMORY, "Could not allocate memory");
        return NULL;
    }
    
    // Load based on format
    bool success = false;
    
    switch (*format) {
        // Geometric formats
        case FORMAT_OBJ:
            *type = DATA_TYPE_GEOMETRIC;
            success = load_obj(file, &data->geometric);
            break;
        case FORMAT_STL:
            *type = DATA_TYPE_GEOMETRIC;
            success = load_stl(file, &data->geometric);
            break;
        case FORMAT_PLY:
            *type = DATA_TYPE_GEOMETRIC;
            success = load_ply(file, &data->geometric);
            break;
            
        // Signal formats
        case FORMAT_WAV:
            *type = DATA_TYPE_SIGNAL;
            success = load_wav(file, &data->signal);
            break;
        case FORMAT_CSV:
            // Could be signal or other data - need to detect
            *type = DATA_TYPE_SIGNAL; // Default to signal
            success = load_csv_signal(file, &data->signal);
            break;
            
        // Image formats
        case FORMAT_JPG:
        case FORMAT_PNG:
        case FORMAT_TIFF:
            *type = DATA_TYPE_IMAGE;
            success = load_image(file, &data->image, *format);
            break;
            
        // Network formats
        case FORMAT_GML:
        case FORMAT_GRAPHML:
            *type = DATA_TYPE_NETWORK;
            success = load_network(file, &data->network, *format);
            break;
            
        // Scientific formats
        case FORMAT_PDB:
            *type = DATA_TYPE_SCIENTIFIC;
            success = load_pdb(file, &data->scientific);
            break;
            
        // ML formats
        case FORMAT_PT:
        case FORMAT_H5:
        case FORMAT_NPY:
            *type = DATA_TYPE_ML;
            success = load_ml(file, &data->ml, *format);
            break;
            
        // JSON (could be any type - need to detect)
        case FORMAT_JSON:
            success = load_json(file, data, type);
            break;
            
        default:
            set_error(ERROR_INVALID_FORMAT, "Format not yet implemented");
            break;
    }
    
    fclose(file);
    
    if (!success) {
        free(data);
        return NULL;
    }
    
    clear_error();
    return data;
}

// ============================================================================
// GENERIC DATA SAVING
// ============================================================================

bool save_data(const char* filename, DataType type, GenericData* data, FileFormat format) {
    if (!filename || !data) {
        set_error(ERROR_INVALID_PARAMETER, "Invalid parameters to save_data");
        return false;
    }
    
    // Open file
    FILE* file = fopen(filename, "wb");
    if (!file) {
        set_error(ERROR_FILE_NOT_FOUND, "Could not create file");
        return false;
    }
    
    bool success = false;
    
    switch (format) {
        // Geometric formats
        case FORMAT_OBJ:
            if (type == DATA_TYPE_GEOMETRIC) {
                success = save_obj(file, &data->geometric);
            }
            break;
        case FORMAT_STL:
            if (type == DATA_TYPE_GEOMETRIC) {
                success = save_stl(file, &data->geometric);
            }
            break;
        case FORMAT_PLY:
            if (type == DATA_TYPE_GEOMETRIC) {
                success = save_ply(file, &data->geometric);
            }
            break;
            
        // Signal formats
        case FORMAT_WAV:
            if (type == DATA_TYPE_SIGNAL) {
                success = save_wav(file, &data->signal);
            }
            break;
        case FORMAT_CSV:
            if (type == DATA_TYPE_SIGNAL) {
                success = save_csv_signal(file, &data->signal);
            }
            break;
            
        // Image formats
        case FORMAT_JPG:
        case FORMAT_PNG:
        case FORMAT_TIFF:
            if (type == DATA_TYPE_IMAGE) {
                success = save_image(file, &data->image, format);
            }
            break;
            
        // Network formats
        case FORMAT_GML:
        case FORMAT_GRAPHML:
            if (type == DATA_TYPE_NETWORK) {
                success = save_network(file, &data->network, format);
            }
            break;
            
        // Scientific formats
        case FORMAT_PDB:
            if (type == DATA_TYPE_SCIENTIFIC) {
                success = save_pdb(file, &data->scientific);
            }
            break;
            
        // ML formats
        case FORMAT_PT:
        case FORMAT_H5:
        case FORMAT_NPY:
            if (type == DATA_TYPE_ML) {
                success = save_ml(file, &data->ml, format);
            }
            break;
            
        // JSON (works for all types)
        case FORMAT_JSON:
            success = save_json(file, data, type);
            break;
            
        default:
            set_error(ERROR_INVALID_FORMAT, "Format not yet implemented");
            break;
    }
    
    fclose(file);
    
    if (!success) {
        return false;
    }
    
    clear_error();
    return true;
}

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

void free_generic_data(DataType type, GenericData* data) {
    if (!data) {
        return;
    }
    
    switch (type) {
        case DATA_TYPE_GEOMETRIC:
            if (data->geometric.vertices) free(data->geometric.vertices);
            if (data->geometric.edges) free(data->geometric.edges);
            if (data->geometric.faces) free(data->geometric.faces);
            if (data->geometric.metadata) free(data->geometric.metadata);
            break;
            
        case DATA_TYPE_SIGNAL:
            if (data->signal.samples) free(data->signal.samples);
            if (data->signal.metadata) free(data->signal.metadata);
            break;
            
        case DATA_TYPE_IMAGE:
            if (data->image.pixels) free(data->image.pixels);
            if (data->image.metadata) free(data->image.metadata);
            break;
            
        case DATA_TYPE_NETWORK:
            if (data->network.nodes) free(data->network.nodes);
            if (data->network.edges) free(data->network.edges);
            if (data->network.properties) free(data->network.properties);
            if (data->network.metadata) free(data->network.metadata);
            break;
            
        case DATA_TYPE_CRYPTO:
            if (data->crypto.hashes) {
                for (size_t i = 0; i < data->crypto.num_hashes; i++) {
                    if (data->crypto.hashes[i]) free(data->crypto.hashes[i]);
                }
                free(data->crypto.hashes);
            }
            if (data->crypto.structure) free(data->crypto.structure);
            if (data->crypto.metadata) free(data->crypto.metadata);
            break;
            
        case DATA_TYPE_SCIENTIFIC:
            if (data->scientific.atoms) free(data->scientific.atoms);
            if (data->scientific.bonds) free(data->scientific.bonds);
            if (data->scientific.states) free(data->scientific.states);
            if (data->scientific.metadata) free(data->scientific.metadata);
            break;
            
        case DATA_TYPE_ML:
            if (data->ml.weights) free(data->ml.weights);
            if (data->ml.layers) free(data->ml.layers);
            if (data->ml.architecture) free(data->ml.architecture);
            if (data->ml.metadata) free(data->ml.metadata);
            break;
            
        default:
            break;
    }
    
    free(data);
}

// ============================================================================
// FORMAT-SPECIFIC LOADERS (Stubs - to be implemented)
// ============================================================================

// These will be implemented in separate files for each format

static bool load_obj(FILE* file, GeometricData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "OBJ loader not yet implemented");
    return false;
}

static bool load_stl(FILE* file, GeometricData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "STL loader not yet implemented");
    return false;
}

static bool load_ply(FILE* file, GeometricData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "PLY loader not yet implemented");
    return false;
}

static bool load_wav(FILE* file, SignalData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "WAV loader not yet implemented");
    return false;
}

static bool load_csv_signal(FILE* file, SignalData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "CSV signal loader not yet implemented");
    return false;
}

static bool load_image(FILE* file, ImageData* data, FileFormat format) {
    (void)file; (void)data; (void)format;
    set_error(ERROR_INVALID_FORMAT, "Image loader not yet implemented");
    return false;
}

static bool load_network(FILE* file, NetworkData* data, FileFormat format) {
    (void)file; (void)data; (void)format;
    set_error(ERROR_INVALID_FORMAT, "Network loader not yet implemented");
    return false;
}

static bool load_pdb(FILE* file, ScientificData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "PDB loader not yet implemented");
    return false;
}

static bool load_ml(FILE* file, MLData* data, FileFormat format) {
    (void)file; (void)data; (void)format;
    set_error(ERROR_INVALID_FORMAT, "ML loader not yet implemented");
    return false;
}

static bool load_json(FILE* file, GenericData* data, DataType* type) {
    (void)file; (void)data; (void)type;
    set_error(ERROR_INVALID_FORMAT, "JSON loader not yet implemented");
    return false;
}

// ============================================================================
// FORMAT-SPECIFIC SAVERS (Stubs - to be implemented)
// ============================================================================

static bool save_obj(FILE* file, GeometricData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "OBJ saver not yet implemented");
    return false;
}

static bool save_stl(FILE* file, GeometricData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "STL saver not yet implemented");
    return false;
}

static bool save_ply(FILE* file, GeometricData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "PLY saver not yet implemented");
    return false;
}

static bool save_wav(FILE* file, SignalData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "WAV saver not yet implemented");
    return false;
}

static bool save_csv_signal(FILE* file, SignalData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "CSV signal saver not yet implemented");
    return false;
}

static bool save_image(FILE* file, ImageData* data, FileFormat format) {
    (void)file; (void)data; (void)format;
    set_error(ERROR_INVALID_FORMAT, "Image saver not yet implemented");
    return false;
}

static bool save_network(FILE* file, NetworkData* data, FileFormat format) {
    (void)file; (void)data; (void)format;
    set_error(ERROR_INVALID_FORMAT, "Network saver not yet implemented");
    return false;
}

static bool save_pdb(FILE* file, ScientificData* data) {
    (void)file; (void)data;
    set_error(ERROR_INVALID_FORMAT, "PDB saver not yet implemented");
    return false;
}

static bool save_ml(FILE* file, MLData* data, FileFormat format) {
    (void)file; (void)data; (void)format;
    set_error(ERROR_INVALID_FORMAT, "ML saver not yet implemented");
    return false;
}

static bool save_json(FILE* file, GenericData* data, DataType type) {
    (void)file; (void)data; (void)type;
    set_error(ERROR_INVALID_FORMAT, "JSON saver not yet implemented");
    return false;
}