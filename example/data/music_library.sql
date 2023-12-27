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

insert into artists(artist_name) values ('Drake and J. Cole');

insert into albums(artist_id, album_year, album_name) values ((select artist_id from artists), 2023, 'First Person Shooter - Single');

insert into songs(artist_id, album_id, song_title) values ((select artist_id from albums), (select album_id from albums), 'First Person Shooter');
