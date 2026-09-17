// Scatter brush: paints random tiles from the current tileset selection at a
// given density within a circular radius, like an airbrush. Tile probability
// weights the pick. [ / ] change the radius, - / = change the density.

tiled.registerTool("ScatterBrush", {
    name: "Scatter Brush",
    usesSelectedTiles: true,

    radius: 2,
    density: 0.15,
    edit: null,
    variants: [],
    visited: null,

    activated() {
        this.updatePreview();
        this.updateStatusInfo();
    },

    mousePressed(button, x, y, modifiers) {
        if (button !== 1) {
            return;
        }
        const layer = this.map?.currentLayer;
        if (!layer || !layer.isTileLayer) {
            return;
        }
        this.variants = this.collectVariants();
        if (this.variants.length === 0) {
            return;
        }
        this.edit = layer.edit();
        this.visited = new Set();
        this.scatter(this.tilePosition.x, this.tilePosition.y);
    },

    mouseReleased(button, x, y, modifiers) {
        if (button === 1) {
            this.edit = null;
            this.visited = null;
        }
    },

    tilePositionChanged(x, y) {
        if (this.edit) {
            this.scatter(x, y);
        }
        this.updatePreview();
        this.updateStatusInfo();
    },

    keyPressed(key, modifiers) {
        switch (key) {
        case 0x5b: this.radius = Math.max(0, this.radius - 1); break;             // [
        case 0x5d: this.radius = Math.min(32, this.radius + 1); break;            // ]
        case 0x2d: this.density = Math.max(0.01, this.density - 0.05); break;     // -
        case 0x3d: this.density = Math.min(1, this.density + 0.05); break;        // =
        default: return;
        }
        this.updatePreview();
        this.updateStatusInfo();
    },

    updateStatusInfo() {
        const pos = this.tilePosition;
        this.statusInfo = `${pos.x}, ${pos.y} - radius ${this.radius}, density ${Math.round(this.density * 100)}%`;
    },

    // null cells in the preview are drawn as a highlight without a tile, which
    // outlines the brush area around the cursor
    updatePreview() {
        const pos = this.tilePosition;
        const r = this.radius;
        const preview = new TileMap();
        const layer = new TileLayer();
        preview.addLayer(layer);
        const edit = layer.edit();
        for (let y = pos.y - r; y <= pos.y + r; ++y) {
            for (let x = pos.x - r; x <= pos.x + r; ++x) {
                if ((x - pos.x) ** 2 + (y - pos.y) ** 2 <= r * r) {
                    edit.setTile(x, y, null);
                }
            }
        }
        edit.apply();
        this.preview = preview;
    },

    // unique tiles in the current brush, skipping the empty cells a
    // non-contiguous tileset selection leaves behind
    collectVariants() {
        const brush = tiled.mapEditor.currentBrush;
        const tiles = new Map();
        for (let i = 0; i < brush.layerCount; ++i) {
            const layer = brush.layerAt(i);
            if (!layer.isTileLayer) {
                continue;
            }
            for (let y = 0; y < layer.height; ++y) {
                for (let x = 0; x < layer.width; ++x) {
                    const tile = layer.tileAt(x, y);
                    if (tile && tile.probability > 0) {
                        tiles.set(`${tile.tileset.name}:${tile.id}`, tile);
                    }
                }
            }
        }
        return [...tiles.values()];
    },

    pickVariant() {
        let roll = Math.random() * this.variants.reduce((sum, tile) => sum + tile.probability, 0);
        for (const tile of this.variants) {
            roll -= tile.probability;
            if (roll < 0) {
                return tile;
            }
        }
        return this.variants[this.variants.length - 1];
    },

    // each cell gets one roll per stroke so dragging back over an area
    // doesn't keep filling it in
    scatter(cx, cy) {
        const layer = this.edit.target;
        const r = this.radius;
        let changed = false;
        for (let y = cy - r; y <= cy + r; ++y) {
            for (let x = cx - r; x <= cx + r; ++x) {
                if ((x - cx) ** 2 + (y - cy) ** 2 > r * r) {
                    continue;
                }
                if (!this.map.infinite && (x < 0 || y < 0 || x >= layer.width || y >= layer.height)) {
                    continue;
                }
                const key = `${x},${y}`;
                if (this.visited.has(key)) {
                    continue;
                }
                this.visited.add(key);
                if (Math.random() < this.density) {
                    this.edit.setTile(x, y, this.pickVariant());
                    changed = true;
                }
            }
        }
        // later applies in the same stroke merge into one undo step
        if (changed) {
            this.edit.apply();
        }
    },
});
