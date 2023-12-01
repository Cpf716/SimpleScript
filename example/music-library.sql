alter user 'root'@'localhost' identified with mysql_native_password by 'root';
flush privileges;

create database if not exists music_library;

use music_library;

drop table if exists songs;
drop table if exists albums;
drop table if exists artists;

create table if not exists artists(
	artist_id int unsigned not null unique auto_increment,
	artist_name varchar(256) not null,
	primary key(artist_id)
);

create table if not exists albums(
	album_id int unsigned not null unique auto_increment,
	artist_id int unsigned not null,
	album_year int unsigned not null,
	album_name varchar(256) not null,
	primary key(album_id),
	foreign key(artist_id) references artists(artist_id)
);

create table if not exists songs(
	song_id int unsigned not null unique auto_increment,
	artist_id int unsigned not null,
	album_id int unsigned not null,
	song_title varchar(256) not null,
	foreign key(album_id) references albums(album_id),
	foreign key(artist_id) references artists(artist_id)
);

insert into artists(artist_name) values ('The Weeknd');

insert into albums(artist_id, album_year, album_name) values ((select artist_id from artists), 2019, 'Blinding Lights - Single');

insert into songs(artist_id, album_id, song_title) values ((select artist_id from albums), (select album_id from albums), 'Blinding Lights');

insert into music_library.albums(artist_id, album_year, album_name) values ((select artist_id from music_library.artists), 2016, 'Starboy - Single')

insert into music_library.songs(artist_id, album_id, song_title) values ((select artist_id from music_library.artists), (select album_id from music_library.albums order by album_id desc limit 1), 'Starboy')

select artist_name, album_year, album_name, song_title from songs join albums on songs.album_id = albums.album_id join artists on albums.artist_id = artists.artist_id;