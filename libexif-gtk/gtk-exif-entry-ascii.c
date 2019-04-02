/* gtk-exif-entry-ascii.c
 *
 * Copyright © 2001 Lutz Müller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "gtk-exif-entry-ascii.h"
#include "gtk-exif-util.h"

#include <string.h>
#include <gtk/gtk.h>

struct _GtkExifEntryAsciiPrivate {
	ExifEntry *entry;

	GtkEntry *gtk_entry;
};

#define PARENT_TYPE GTK_EXIF_TYPE_ENTRY
static GtkExifEntryClass *parent_class;

static void
#if GTK_CHECK_VERSION(3,0,0)
gtk_exif_entry_ascii_destroy (GtkWidget *widget)
#else
gtk_exif_entry_ascii_destroy (GtkObject *object)
#endif
{
#if GTK_CHECK_VERSION(3,0,0)
	GtkExifEntryAscii *entry = GTK_EXIF_ENTRY_ASCII (widget);
#else
	GtkExifEntryAscii *entry = GTK_EXIF_ENTRY_ASCII (object);
#endif

	if (entry->priv->entry) {
		exif_entry_unref (entry->priv->entry);
		entry->priv->entry = NULL;
	}

#if GTK_CHECK_VERSION(3,0,0)
	GTK_WIDGET_CLASS (parent_class)->destroy (widget);
#else
	GTK_OBJECT_CLASS (parent_class)->destroy (object);
#endif
}

GTK_EXIF_FINALIZE (entry_ascii, EntryAscii)

static void
gtk_exif_entry_ascii_class_init (gpointer g_class, gpointer class_data G_GNUC_UNUSED)
{
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidgetClass *widget_class;
	GObjectClass *gobject_class;

	widget_class = GTK_WIDGET_CLASS (g_class);
	widget_class->destroy  = gtk_exif_entry_ascii_destroy;
#else
	GtkObjectClass *object_class;
	GObjectClass *gobject_class;

	object_class = GTK_OBJECT_CLASS (g_class);
	object_class->destroy  = gtk_exif_entry_ascii_destroy;
#endif

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gtk_exif_entry_ascii_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gtk_exif_entry_ascii_init (GTypeInstance *instance, gpointer g_class G_GNUC_UNUSED)
{
	GtkExifEntryAscii *entry = GTK_EXIF_ENTRY_ASCII (instance);

	entry->priv = g_new0 (GtkExifEntryAsciiPrivate, 1);
}

GTK_EXIF_CLASS (entry_ascii, EntryAscii, "EntryAscii")

static void
on_text_changed (GtkEditable *editable, GtkExifEntryAscii *entry)
{
	gchar *txt;

	txt = gtk_editable_get_chars (editable, 0, -1);
	g_free (entry->priv->entry->data);
	entry->priv->entry->data = (unsigned char *)txt;
	entry->priv->entry->size = strlen (txt) + 1;
	entry->priv->entry->components = entry->priv->entry->size;
	gtk_exif_entry_changed (GTK_EXIF_ENTRY (entry), entry->priv->entry);
}

GtkWidget *
gtk_exif_entry_ascii_new (ExifEntry *e)
{
	GtkExifEntryAscii *entry;
	GtkWidget *widget;

	g_return_val_if_fail (e != NULL, NULL);
	g_return_val_if_fail (e->format == EXIF_FORMAT_ASCII, NULL);

	entry = g_object_new (GTK_EXIF_TYPE_ENTRY_ASCII, NULL);
	entry->priv->entry = e;
	exif_entry_ref (e);
	gtk_exif_entry_construct (GTK_EXIF_ENTRY (entry),
		exif_tag_get_title_in_ifd (e->tag, exif_content_get_ifd(e->parent)),
		exif_tag_get_description_in_ifd (e->tag, exif_content_get_ifd(e->parent)));

	widget = gtk_entry_new ();
	gtk_widget_show (widget);
	gtk_box_pack_start (GTK_BOX (entry), widget, TRUE, FALSE, 0);
	gtk_entry_set_text (GTK_ENTRY (widget), (gchar *)e->data);
	g_signal_connect (G_OBJECT (widget), "changed",
			  G_CALLBACK (on_text_changed), entry);

	return (GTK_WIDGET (entry));
}
