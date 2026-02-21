#!/bin/bash
# ══════════════════════════════════════════════════
#  PRIMUS AI — Database Loader
#  Usage: bash load_db.sh
# ══════════════════════════════════════════════════

DB="/home/pi/primus/ml/knowledge.db"
SQL="/home/pi/primus/ml/seed_knowledge_v3.sql"

echo "══════════════════════════════════════════════════"
echo "  📚 PRIMUS AI — Database Loader v3.0"
echo "══════════════════════════════════════════════════"

# Backup old DB if exists
if [ -f "$DB" ]; then
    cp "$DB" "${DB}.bak"
    echo "✅ Old DB backed up to knowledge.db.bak"
fi

# Drop and recreate
rm -f "$DB"
echo "🔨 Creating fresh database..."

sqlite3 "$DB" << 'SQL'
PRAGMA encoding = "UTF-8";
PRAGMA journal_mode = WAL;
PRAGMA synchronous = NORMAL;
PRAGMA cache_size = -64000;
PRAGMA temp_store = MEMORY;
PRAGMA mmap_size = 268435456;

CREATE TABLE knowledge (
    id       INTEGER PRIMARY KEY AUTOINCREMENT,
    question TEXT    NOT NULL,
    answer   TEXT    NOT NULL,
    category TEXT    NOT NULL DEFAULT 'सामान्य'
);

CREATE INDEX idx_category ON knowledge(category);
CREATE INDEX idx_cat_q    ON knowledge(category, question);
SQL

echo "📥 Loading entries..."
sqlite3 "$DB" < "$SQL"

echo "⚡ Optimizing..."
sqlite3 "$DB" "ANALYZE; VACUUM;"

COUNT=$(sqlite3 "$DB" "SELECT COUNT(*) FROM knowledge;")
echo ""
echo "══════════════════════════════════════════════════"
echo "  ✅ Done! $COUNT entries loaded"
echo "══════════════════════════════════════════════════"
sqlite3 "$DB" "SELECT category, COUNT(*) as n FROM knowledge GROUP BY category ORDER BY n DESC;"
